#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <gl\glaux.h>
#include <stdio.h>
#include <math.h>
#include <glut.h>

HDC			hDC = NULL, hDC1=NULL;		// Private GDI Device Context
HGLRC		hRC = NULL, hRC1=NULL;		// Permanent Rendering Context
HWND		hWnd0 = NULL, hWnd1=NULL;		// Holds Our Window Handle
HINSTANCE	hInstance, hIns;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = FALSE;	// Fullscreen Flag Set To Fullscreen Mode By Default
bool	blend;				// Blending ON/OFF
bool	bp;					// B Pressed?
bool	fp,rp, aim=FALSE;					// F Pressed?
bool shot = FALSE,bullet_fly=FALSE;
float test; 

typedef struct 
{
	float x_cor[10000], y_cor[10000], z_cor[10000];
	float line_x[10000], line_y[10000], line_z[10000];
	GLfloat time[10000], speed[10000], speed_line[10000];
	float x_track, y_track;
	bool fly;
	float decel;
	int layer;
} bullet;
bullet bullet1;

/*float lx = x*cosf(angle);
float ly = x*sinf(angle);
float lz = z0;*/

const float piover180 = 0.0174532925f, d_time=0.0015, const_b_speed=21.65, const_k_speed=1.01;
float heading, xs,xe,ys,ye,zs,ze,ggg;
float xpos;
float zpos;
float target_z = -20, target_size = 1, target_x = -target_size/2, target_y=0;
int num_wind = 1, norm_temp=15, temp, temp_coef,norm_press=750,press,press_coef;
float aimx=0, aimy=0.18, aimz=-10.,
back_color_r = 0.1058f, back_color_g = 0.8f, back_color_b = 0.9216f;
  
GLfloat	yrot;				// Y Rotation
GLfloat walkbias = 0;
GLfloat walkbiasangle = 0;
GLfloat lookupdown = 0.065f;
GLfloat	z = 0.0f;				// Depth Into The Screen

GLuint	filter;				// Which Filter To Use
GLuint	texture[10];			// Storage For 3 Textures

WNDCLASS	wc,wc1;

typedef struct tagVERTEX
{
	float x, y, z;
	float u, v;
} VERTEX;

typedef struct tagTRIANGLE
{
	VERTEX vertex[4];
	int tex;
} TRIANGLE;

typedef struct tagSECTOR
{
	int numtriangles;
	TRIANGLE* triangle;
} SECTOR;

SECTOR sector1;				// Our Model Goes Here:

typedef struct wind_character1
{
	float angle;
	float speed;
} wind_character;
wind_character wind;

void setup_wind()
{
	float i, j,k;
	FILE *in;
	in = fopen("wind.txt", "r");
	fscanf(in, "%f %f", &i, &j);
	fscanf(in, "%f",&k);
	temp = k;
	fscanf(in, "%f", &k);
	press = k;
	fclose(in);
	
	wind.angle = i;
	wind.speed = j*0.1;
}
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

void readstr(FILE *f, char *string)
{
	do
	{
		fgets(string, 255, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
	return;
}

void SetupWorld()
{
	bullet1.layer = 0;
	bullet1.fly = FALSE;

	float x, y, z, u, v;
	int tex;
	int numtriangles;
	FILE *filein;
	char oneline[255];

	setup_wind();

	filein = fopen("world10.txt", "rt");				// File To Load World Data From

	//readstr(filein, oneline);
	//sscanf(oneline, "%d\n", &numtriangles);
	fscanf(filein, "%f %f %f %f %f %f", &xs, &xe, &ys, &ye, &zs, &ze);
	fscanf(filein, "%d", &numtriangles);
//NUMPOLLIES 
	sector1.triangle = new TRIANGLE[numtriangles];
	sector1.numtriangles = numtriangles;
	for (int loop = 0; loop < numtriangles; loop++)
	{
		fscanf(filein, "%d", &tex);
		for (int vert = 0; vert < 4; vert++)
		{
			//readstr(filein, oneline);
			//sscanf(oneline, "%f %f %f %f %f %d", &x, &y, &z, &u, &v, &tex);
			fscanf(filein, "%f %f %f %f %f", &x, &y, &z, &u, &v);
			sector1.triangle[loop].vertex[vert].x = x;
			sector1.triangle[loop].vertex[vert].y = y;
			sector1.triangle[loop].vertex[vert].z = z;
			sector1.triangle[loop].vertex[vert].u = u;
			sector1.triangle[loop].vertex[vert].v = v;
			sector1.triangle[loop].tex = tex-1;
		}
	}
	fclose(filein);
	return;
}

int find_aim(float zzz)
{
	int i = 0;
	while (i < bullet1.layer && bullet1.z_cor[i] > zzz)
		i++;
	if (i == bullet1.layer)
		return -1;
	else
		return i;
}

void print_bullet_character(bullet bull)
{
	FILE *in;
	in = fopen("bullet.txt", "w");
	for (int i = 0; i < bull.layer; i++)
	{
		//смешение по х
		//fprintf(in, "%f\t%f\t%f\t%f\t%f\n", -bull.line_z[i] * 10, bull.line_x[i] * 10, bull.line_y[i] * 10,
			//bull.x_cor[i] * 10, bull.y_cor[i] * 10/*, bull.z_cor[i] * 10, bull.speed[i] * 10*/);

		//смешение по z
		fprintf(in, "%f\t%f\t%f\n", /*-bull.line_z[i] * 10, bull.line_x[i] * 10, bull.line_y[i] * 10, bull.speed_line[i] * 10,*/
			-bull.z_cor[i] * 10, /*bull.x_cor[i] * 10, bull.y_cor[i] * 10,*/ bull.speed[i] * 10, bull.time[i]);

		
		//fprintf(in, "%f\t%f\t%f\t%f\t%f\t%f\t%f\n", bull.line_x[i] * 10, bull.line_y[i] * 10, bull.line_z[i] * 10,
			//bull.x_cor[i] * 10, bull.y_cor[i] * 10, bull.z_cor[i] * 10, bull.speed[i] * 10);
	}
	//fprintf(in, "%f", bull.time);
	fclose(in);
}

AUX_RGBImageRec *LoadBMP(char *Filename)                // Loads A Bitmap Image
{
	FILE *File = NULL;                                // File Handle

	if (!Filename)                                  // Make Sure A Filename Was Given
	{
		return NULL;                            // If Not Return NULL
	}

	File = fopen(Filename, "r");                       // Check To See If The File Exists

	if (File)                                       // Does The File Exist?
	{
		fclose(File);                           // Close The Handle
		return auxDIBImageLoad(Filename);       // Load The Bitmap And Return A Pointer
	}
	return NULL;                                    // If Load Failed Return NULL
}

int LoadGLTextures()                                    // Load Bitmaps And Convert To Textures
{
	int Status = FALSE;                               // Status Indicator

	AUX_RGBImageRec *TextureImage[1];               // Create Storage Space For The Texture

	memset(TextureImage, 0, sizeof(void *)* 1);        // Set The Pointer To NULL

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if (TextureImage[0] = LoadBMP("sand2.bmp"))
	{
		Status = TRUE;                            // Set The Status To TRUE

		glGenTextures(1, &texture[0]);          // Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);

		// Create Linear Filtered Texture
		/*glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);*/
	}
	if (TextureImage[0])                            // If Texture Exists
	{
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure
	}

	if (TextureImage[0] = LoadBMP("white.bmp"))
	{
		Status = TRUE;                            // Set The Status To TRUE

		glGenTextures(1, &texture[1]);          // Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
	}
	if (TextureImage[0])                            // If Texture Exists
	{
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure
	}
	if (TextureImage[0] = LoadBMP("sky.bmp"))
	{
		Status = TRUE;                            // Set The Status To TRUE

		glGenTextures(1, &texture[2]);          // Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
	}
	if (TextureImage[0])                            // If Texture Exists
	{
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure
	}

	if (TextureImage[0] = LoadBMP("target.bmp"))
	{
		Status = TRUE;                            // Set The Status To TRUE

		glGenTextures(1, &texture[3]);          // Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[3]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
	}
	if (TextureImage[0])                            // If Texture Exists
	{
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure
	}

	if (TextureImage[0] = LoadBMP("red.bmp"))
	{
		Status = TRUE;                            // Set The Status To TRUE

		glGenTextures(1, &texture[4]);          // Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
	}
	if (TextureImage[0])                            // If Texture Exists
	{
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure
	}

	if (TextureImage[0] = LoadBMP("green.bmp"))
	{
		Status = TRUE;                            // Set The Status To TRUE

		glGenTextures(1, &texture[5]);          // Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[5]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
	}
	if (TextureImage[0])                            // If Texture Exists
	{
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure
	}

	if (TextureImage[0] = LoadBMP("black.bmp"))
	{
		Status = TRUE;                            // Set The Status To TRUE

		glGenTextures(1, &texture[6]);          // Create Three Textures

		// Create Nearest Filtered Texture
		glBindTexture(GL_TEXTURE_2D, texture[6]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
	}
	if (TextureImage[0])                            // If Texture Exists
	{
		if (TextureImage[0]->data)              // If Texture Image Exists
		{
			free(TextureImage[0]->data);    // Free The Texture Image Memory
		}

		free(TextureImage[0]);                  // Free The Image Structure
	}
	return Status;                                  // Return The Status
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 200.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

/*void mouseFunc(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && !bullet_fly) {

		// when the button is released
		if (state == GLUT_DOWN) 
		{
			shot = !shot;
		}
	}
}


void mouseButton(int button, int state, int x, int y)
{
	// only start motion if the left button is pressed 
	if (button == GLUT_LEFT_BUTTON)
	{
		// when the button is released 
		if (state == GLUT_UP)
		{
			shot = TRUE;
			//xOrigin = -1;
		}
		else
		{// state = GLUT_DOWN 
			//xOrigin = x;
		}
		//initVBO();
	}
}*/

int InitGL1(GLvoid)										// All Setup For OpenGL Goes Here
{
	

	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);					// Set The Blending Function For Translucency
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// This Will Clear The Background Color To Black
	glClearDepth(1.0);									// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);								// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glShadeModel(GL_SMOOTH);							// Enables Smooth Color Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	//glutMouseFunc(mouseFunc);

	

	return TRUE;										// Initialization Went OK
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	if (!LoadGLTextures())								// Jump To Texture Loading Routine
	{
		return FALSE;									// If Texture Didn't Load Return FALSE
	}
	
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);					// Set The Blending Function For Translucency
	glClearColor(0.1058f, 0.8f, 0.9216f, 0.0f);				// This Will Clear The Background Color To Black
	glClearDepth(1.0);									// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);								// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glShadeModel(GL_SMOOTH);							// Enables Smooth Color Shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	//glutMouseFunc(mouseFunc);

	SetupWorld();

	return TRUE;										// Initialization Went OK
}

int DrawGLScene1(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();

	
	//поле для рисования
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex3f(-1.2, -0.6, -2);

	glTexCoord2f(0, 1);
	glVertex3f(-1.2, 0.6, -2);

	glTexCoord2f(1, 1);
	glVertex3f(1.2, 0.6, -2);

	glTexCoord2f(1, 0);
	glVertex3f(1.2, -0.6, -2);

	glEnd();
	//------------
	//оси
	//х
	glLineWidth(6);
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glBegin(GL_LINE_STRIP);
	glTexCoord2f(0, 0);
	glVertex3f(-1.2 - 0.014, -0.6 - 0.006, -2);
	glTexCoord2f(1, 0);
	glVertex3f(1.4, -0.6 - 0.006, -2);
	glEnd();

	//y
	glBegin(GL_LINE_STRIP);
	glTexCoord2f(0, 0);
	glVertex3f(-1.2 - 0.006, -0.6, -2);
	glTexCoord2f(1, 0);
	glVertex3f(-1.2 - 0.006, 0.8, -2);
	glEnd();
	//шкала
	glLineWidth(3);
	for (int i = 1; i <= 10; i++)//х, она же z
	{
		glBegin(GL_LINE_STRIP);
		glVertex3f(-1.2 + 0.24*i, -0.6 + 0.015, -1.999);
		glVertex3f(-1.2 + 0.24*i, -0.6 - 0.025, -1.999);
		glEnd();
	}
	for (int i = 1; i <= 5; i++)//y
	{
		glBegin(GL_LINE_STRIP);
		glVertex3f(-1.2 + 0.015, -0.6 + 0.24*i, -1.999);
		glVertex3f(-1.2 - 0.025, -0.6 + 0.24*i, -1.999);
		glEnd();
	}
	
	//траектория
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < bullet1.layer; i += 10)
	{
		glVertex3d(-1.2 - bullet1.z_cor[i]/100*2.4, -0.6 + bullet1.y_cor[i]*1.2*2, -1.999);
	}
	glEnd();
	return TRUE;

}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View

	GLfloat x_m, y_m, z_m, u_m, v_m;
	GLfloat xtrans = -xpos;
	GLfloat ztrans = -zpos;
	GLfloat ytrans = /*-walkbias*/ - 0.18f;
	GLfloat sceneroty = 360.0f - yrot;

	int numtriangles,tex;

	glRotatef(lookupdown, 1.0f, 0, 0);
	glRotatef(sceneroty, 0, 1.0f, 0);

	glTranslatef(xtrans, ytrans, ztrans);
	

	numtriangles = sector1.numtriangles;

	// Process Each Triangle
	for (int loop_m = 0; loop_m < numtriangles; loop_m++)
	{
		tex = sector1.triangle[loop_m].tex;
		if ( sector1.triangle[loop_m].vertex[1].y < 0.5)
		{
			glBindTexture(GL_TEXTURE_2D, texture[tex]);
			glBegin(GL_QUADS);
			glNormal3f(0.0f, 0.0f, 1.0f);
			x_m = sector1.triangle[loop_m].vertex[0].x;
			y_m = sector1.triangle[loop_m].vertex[0].y;
			z_m = sector1.triangle[loop_m].vertex[0].z;
			u_m = sector1.triangle[loop_m].vertex[0].u;
			v_m = sector1.triangle[loop_m].vertex[0].v;
			glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

			x_m = sector1.triangle[loop_m].vertex[1].x;
			y_m = sector1.triangle[loop_m].vertex[1].y;
			z_m = sector1.triangle[loop_m].vertex[1].z;
			u_m = sector1.triangle[loop_m].vertex[1].u;
			v_m = sector1.triangle[loop_m].vertex[1].v;
			glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

			x_m = sector1.triangle[loop_m].vertex[2].x;
			y_m = sector1.triangle[loop_m].vertex[2].y;
			z_m = sector1.triangle[loop_m].vertex[2].z;
			u_m = sector1.triangle[loop_m].vertex[2].u;
			v_m = sector1.triangle[loop_m].vertex[2].v;
			glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

			x_m = sector1.triangle[loop_m].vertex[3].x;
			y_m = sector1.triangle[loop_m].vertex[3].y;
			z_m = sector1.triangle[loop_m].vertex[3].z;
			u_m = sector1.triangle[loop_m].vertex[3].u;
			v_m = sector1.triangle[loop_m].vertex[3].v;
			glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

			glEnd();
		}
	}
	//glColor3f(back_color_r, back_color_g, back_color_b);
	
	//мишень
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	
	for (int i = 1; i <= 10; i++)
	{
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		x_m = target_x;
		y_m = target_y;
		z_m = -i*10.0; //target_z;
		u_m = 0;
		v_m = 0;
		glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

		x_m = target_x;
		y_m = target_y + target_size;
		z_m = -i*10.0; //target_z;
		u_m = 0;
		v_m = 1;
		glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

		x_m = target_x + target_size;
		y_m = target_y + target_size;
		z_m = -i*10.0; //target_z;
		u_m = 1;
		v_m = 1;
		glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

		x_m = target_x + target_size;
		y_m = target_y;
		z_m = -i*10.0; //target_z;
		u_m = 1;
		v_m = 0;
		glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

		glEnd();
	}

	

	glLineWidth(3);
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glBegin(GL_LINE_STRIP);
	//glColor3d(1., 0., 0.);
	for (int i = 0; i < bullet1.layer; i++)
	{
		glTexCoord2f(0, 1);
		glVertex3d(bullet1.x_cor[i], bullet1.y_cor[i], bullet1.z_cor[i]);
	}
	glEnd();
	//зеленая траектория
	/*
	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glBegin(GL_LINE_STRIP);
	//glColor3d(1., 0., 0.);
	for (int i = 0; i < bullet1.layer; i++)
	{
		glTexCoord2f(0, 1);
		glVertex3d(bullet1.line_x[i], bullet1.line_y[i], bullet1.line_z[i]);
	}
	glEnd();
	*/
	//прицел
	if (aim)
	{
		glBindTexture(GL_TEXTURE_2D, texture[6]);

		glBegin(GL_LINE_STRIP);

		glTexCoord2f(0, 1);
		glVertex3d(bullet1.x_cor[0], bullet1.y_cor[0]+0.007, bullet1.z_cor[0]);
		glTexCoord2f(0, 1);
		glVertex3d(aimx,aimy,aimz);
		glTexCoord2f(0, 1);
		glVertex3d(aimx, (aimy - bullet1.y_cor[0]-0.007) / (aimz - bullet1.z_cor[0]) *(-100 - bullet1.z_cor[0]) + bullet1.y_cor[0]+0.007 , -100);

		glEnd();
	}

	return TRUE;										// Everything Went OK
}

GLvoid KillGLWindow1(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC1)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC1))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC1 = NULL;										// Set RC To NULL
	}

	if (hDC1 && !ReleaseDC(hWnd1, hDC1))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC1 = NULL;										// Set DC To NULL
	}

	if (hWnd1 && !DestroyWindow(hWnd1))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd1 = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL1", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd0, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd0 && !DestroyWindow(hWnd0))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd0 = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGlGrafics()
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	//WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)500;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)700;		// Set Bottom Value To Requested Height

	fullscreen = FALSE;			// Set The Global Fullscreen Flag

	 hIns = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc1.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc1.lpfnWndProc = (WNDPROC)WndProc/*(hWnd1,)*/;					// WndProc Handles Messages
	wc1.cbClsExtra = 0;									// No Extra Window Data
	wc1.cbWndExtra = 0;									// No Extra Window Data
	wc1.hInstance = hIns;							// Set The Instance
	wc1.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc1.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc1.hbrBackground = NULL;									// No Background Required For GL
	wc1.lpszMenuName = NULL;									// We Don't Want A Menu
	wc1.lpszClassName = "OpenGL1";								// Set The Class Name

	if (!RegisterClass(&wc1))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class1.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = 500;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = 700;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = 16;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size


	char* title = "grafics";
	// Create The Window
	if (!(hWnd1 = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL1",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,	//hPar;							// No Parent Window
		NULL,								// No Menu
		hIns,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow1();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		16,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC1 = GetDC(hWnd1)))							// Did We Get A Device Context?
	{
		KillGLWindow1();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC1, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow1();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC1, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow1();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC1 = wglCreateContext(hDC1)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow1();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC1, hRC1))					// Try To Activate The Rendering Context
	{
		KillGLWindow1();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd1, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd1);						// Slightly Higher Priority
	SetFocus(hWnd1);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(500, 700);					// Set Up Our Perspective GL Screen

	if (!InitGL1())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow1();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;
}

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag, int wind )
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	//WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size



	// Create The Window
	if (!(hWnd0 = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd0)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd0, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd0);						// Slightly Higher Priority
	SetFocus(hWnd0);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
	{
													if (!HIWORD(wParam))					// Check Minimization State
													{
														active = TRUE;						// Program Is Active
													}
													else
													{
														active = FALSE;						// Program Is No Longer Active
													}

													return 0;								// Return To The Message Loop
	}

	case WM_SYSCOMMAND:							// Intercept System Commands
	{
													switch (wParam)							// Check System Calls
													{
													case SC_SCREENSAVE:					// Screensaver Trying To Start?
													case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
														return 0;							// Prevent From Happening
													}
													break;									// Exit
	}

	case WM_CLOSE:								// Did We Receive A Close Message?
	{
													PostQuitMessage(0);						// Send A Quit Message
													return 0;								// Jump Back
	}

	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
													keys[wParam] = TRUE;					// If So, Mark It As TRUE
													return 0;								// Jump Back
	}

	case WM_KEYUP:								// Has A Key Been Released?
	{
													keys[wParam] = FALSE;					// If So, Mark It As FALSE
													return 0;								// Jump Back
	}

	case WM_SIZE:								// Resize The OpenGL Window
	{
													ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
													return 0;								// Jump Back
	}
	case WMSZ_BOTTOMLEFT:
	{
							//keys['R'] = TRUE;
							return 0;
	}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done = FALSE;								// Bool Variable To Exit Loop
	HWND par;
	
	//glutInit(&argc, argv);
	//glutMouseFunc(mouseButton);
	// Ask The User Which Screen Mode They Prefer
	/*if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		fullscreen = FALSE;							// Windowed Mode
	}*/

	// Create Our OpenGL Window
	
	
	if (!CreateGLWindow("Lionel Brits & NeHe's 3D World Tutorial", 1200, 700, 16, fullscreen,0))
	{
		return 0;									// Quit If Window Was Not Created
	}/*
if (!CreateGlGrafics())
	{
		return 0;						// Quit If Window Was Not Created
	}*/
	float windy,time_z=0.01584,dz,aa1,aa2,aa3,speed;
	while (!done)									// Loop That Runs While done=FALSE
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				return TRUE;
			}
		}
		/*if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}*/
		//else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if (num_wind == 1)
			{

				if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
				{
					done = TRUE;							// ESC or DrawGLScene Signalled A Quit
				}
				else									// Not Time To Quit, Update Screen
				{
					//Sleep(d_time);
					// Swap Buffers (Double Buffering)
					//SwapBuffers(hDC1);
					SwapBuffers(hDC);
					if (keys['R'] && !rp || shot)
					{
						shot = TRUE;
						rp = TRUE;
						if (!bullet1.fly)
						{
							temp_coef = (temp - norm_temp) / 10;
							press_coef = (press - norm_press) / 10;

							bullet1.fly = !bullet1.fly;
							bullet1.x_track = heading;
							bullet1.y_track = lookupdown;

							bullet1.x_cor[0] = xpos;
							bullet1.y_cor[0] = 0.18f/* + walkbias*/;
							bullet1.z_cor[0] = zpos;

							bullet1.line_x[0] = xpos;
							bullet1.line_y[0] = 0.18;
							bullet1.line_z[0] = zpos;

							bullet1.time[0] = 0;
							bullet1.speed[0] = 83 + 0.8*temp_coef + 0.15*press_coef;
							bullet1.speed_line[0] = 83;
							bullet1.decel = 1;
							bullet1.layer = 1;

							
						}
						else
						{

							bullet1.time[bullet1.layer] = bullet1.time[bullet1.layer - 1] + d_time;
							bullet1.speed[bullet1.layer] = (bullet1.speed[0] - (const_b_speed + (0.8*temp_coef + 0.15*press_coef)))*exp(-(const_k_speed)*bullet1.time[bullet1.layer])
								+ const_b_speed + (0.8*temp_coef + 0.15*press_coef);
							speed = (bullet1.speed[bullet1.layer] + bullet1.speed[bullet1.layer - 1]) / 2;
							test = (float)cos(wind.angle*piover180)*wind.speed;
							bullet1.speed_line[bullet1.layer] = (bullet1.speed_line[0] - const_b_speed)*exp(-const_k_speed*bullet1.time[bullet1.layer]) + const_b_speed;

							//учет внешних сил
							bullet1.x_cor[bullet1.layer] = (float)sin(wind.angle*piover180)*wind.speed*d_time*(1 -  bullet1.speed[bullet1.layer]/bullet1.speed[0])*0.9//ветер
								+ bullet1.x_cor[bullet1.layer - 1] - (float)sin(bullet1.x_track*piover180) * (speed)* d_time //скорость пули
								//+ (float)cos(bullet1.x_track*piover180) *(1 - bullet1.speed[bullet1.layer] / bullet1.speed[0]) / 19 * d_time //деривация
								;
							
							bullet1.z_cor[bullet1.layer] = -(float)cos(wind.angle*piover180)*wind.speed*d_time*(1 -  bullet1.speed[bullet1.layer]/bullet1.speed[0])*0.6//ветер
								+ bullet1.z_cor[bullet1.layer - 1] - (float)cos(bullet1.x_track*piover180) * (speed) * d_time
								//- (float)sin(bullet1.x_track*piover180) *(1 - bullet1.speed[bullet1.layer] / bullet1.speed[0]) / 19 * d_time //деривация
								;
							
							ggg = (float)sin(-bullet1.y_track*piover180);
							bullet1.y_cor[bullet1.layer] = bullet1.y_cor[bullet1.layer - 1] + (ggg * (speed)//* bullet1.time
								-0.98066 * bullet1.time[bullet1.layer]) * d_time
								;
							//линейное движение
							test = (float)sin(bullet1.x_track*piover180);
							bullet1.line_x[bullet1.layer] = //(float)sin(wind.angle*piover180)*wind.speed*d_time*(1 - bullet1.speed_line[bullet1.layer] / bullet1.speed_line[0]) +
								 bullet1.line_x[bullet1.layer - 1] - (float)sin(bullet1.x_track*piover180) * (bullet1.speed_line[bullet1.layer - 1]) * d_time;
							
							bullet1.line_z[bullet1.layer] = //-(float)cos(wind.angle*piover180)*wind.speed*d_time*(1 - bullet1.speed_line[bullet1.layer] / bullet1.speed_line[0])*0.6
								+bullet1.line_z[bullet1.layer - 1] - (float)cos(bullet1.x_track*piover180) * (bullet1.speed_line[bullet1.layer - 1] + bullet1.speed_line[bullet1.layer])/2 * d_time;
							
							bullet1.line_y[bullet1.layer] = bullet1.line_y[bullet1.layer - 1] + (ggg * (bullet1.speed_line[bullet1.layer])//* bullet1.time
								- 0.98066  * bullet1.time[bullet1.layer]) * d_time;

							bullet1.decel++;
							//сравнение продольного ветра
							/*bullet1.line_z[bullet1.layer] = bullet1.z_cor[bullet1.layer];
							dz = bullet1.line_z[bullet1.layer] - bullet1.line_z[bullet1.layer-1];
							time_z = dz/( -(float)cos(bullet1.x_track*piover180) * (bullet1.speed[bullet1.layer - 1]) );
							bullet1.line_y[bullet1.layer] = bullet1.line_y[bullet1.layer - 1] + (ggg * (bullet1.speed[bullet1.layer - 1])//* bullet1.time
							- 0.98  * time_z) * time_z;
							*/
							if (bullet1.x_cor[bullet1.layer] > xe || bullet1.x_cor[bullet1.layer]<xs ||
								bullet1.y_cor[bullet1.layer]>ye || bullet1.y_cor[bullet1.layer]<ys ||
								bullet1.z_cor[bullet1.layer]>ze || bullet1.z_cor[bullet1.layer] < zs-10)
							{
								bullet1.fly = FALSE;
								shot = !shot;
								print_bullet_character(bullet1);
							}

							bullet1.layer++;
						}
					}
					if (!keys['R'])
					{
						rp = FALSE;
					}

					if (keys['B'] && !bp)
					{
						bp = TRUE;
						blend = !blend;
						if (!blend)
						{
							glDisable(GL_BLEND);
							glEnable(GL_DEPTH_TEST);
						}
						else
						{
							glEnable(GL_BLEND);
							glDisable(GL_DEPTH_TEST);
						}
					}


					if (!keys['B'])
					{
						bp = FALSE;
					}
					if (keys['N'])
					{
						/*SetFocus(hWnd1);
						num_wind = 2;*/
						aim = !aim;
						keys['N'] = FALSE;
					}
					if (keys['G'])
					{
						num_wind = 2;
						keys['G'] = FALSE;
					}
					if (keys['1'])
					{
						aimz = -10;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1] 
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['1'] = FALSE;	
					}
					if (keys['2'])
					{
						aimz = -20;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['2'] = FALSE;
					}
					if (keys['3'])
					{
						aimz = -30;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['3'] = FALSE;
					}
					if (keys['4'])
					{
						aimz = -40;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['4'] = FALSE;
					}
					if (keys['5'])
					{
						aimz = -50;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['5'] = FALSE;
					}
					if (keys['6'])
					{
						aimz = -60;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['6'] = FALSE;
					}
					if (keys['7'])
					{
						aimz = -70;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['7'] = FALSE;
					}
					if (keys['8'])
					{
						aimz = -80;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['8'] = FALSE;
					}
					if (keys['9'])
					{
						aimz = -90;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['9'] = FALSE;
					}
					if (keys['0'])
					{
						aimz = -100;
						int i = find_aim(aimz);
						if (i >= 0)
						{
							aimy = bullet1.y_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.y_cor[i - 1] - bullet1.y_cor[i]);
							aimx = bullet1.x_cor[i - 1]
								- (bullet1.z_cor[i - 1] - aimz) / (bullet1.z_cor[i - 1] - bullet1.z_cor[i])*(bullet1.x_cor[i - 1] - bullet1.x_cor[i]);
						}
						keys['0'] = FALSE;
					}
					

					if (keys[VK_UP])
					{

						xpos -= (float)sin(heading*piover180) * 0.5f;
						zpos -= (float)cos(heading*piover180) * 0.5f;
						if (walkbiasangle >= 359.0f)
						{
							walkbiasangle = 0.0f;
						}
						else
						{
							walkbiasangle += 10;
						}
						walkbias = (float)sin(walkbiasangle * piover180) / 20.0f;
					}

					if (keys[VK_DOWN])
					{
						xpos += (float)sin(heading*piover180) * 0.05f;
						zpos += (float)cos(heading*piover180) * 0.05f;
						if (walkbiasangle <= 1.0f)
						{
							walkbiasangle = 359.0f;
						}
						else
						{
							walkbiasangle -= 10;
						}
						walkbias = (float)sin(walkbiasangle * piover180) / 20.0f;
					}

					if (keys[VK_RIGHT])
					{
						heading -= 1.0f;
						yrot = heading;
					}

					if (keys[VK_LEFT])
					{
						heading += 1.0f;
						yrot = heading;
					}

					if (keys[VK_PRIOR])
					{
						lookupdown -= 0.135f;
					}

					if (keys[VK_NEXT])
					{
						lookupdown += 1.0f;
					}

					if (keys[VK_F1])						// Is F1 Being Pressed?
					{
						keys[VK_F1] = FALSE;					// If So Make Key FALSE
						//					glutFullScreen();
						/*KillGLWindow();						// Kill Our Current Window
						fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
						// Recreate Our OpenGL Window
						if (!CreateGLWindow("Lionel Brits & NeHe's 3D World Tutorial", 640, 480, 16, fullscreen,0))
						{
						return 0;						// Quit If Window Was Not Created
						}*/
					}
				}
			}
			else
			{
				if ((active && !DrawGLScene1()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
				{
					done = TRUE;							// ESC or DrawGLScene Signalled A Quit
				}
				else									// Not Time To Quit, Update Screen
				{
					Sleep(15);
					// Swap Buffers (Double Buffering)
					SwapBuffers(hDC);
					if (keys['G'])
					{
						//SetFocus(hWnd0);
						num_wind = 1;
						keys['G'] = FALSE;
					}
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();										// Kill The Window
	//KillGLWindow1();
	return (msg.wParam);								// Exit The Program
}
#include <ctime>
#include <stdlib.h>
#include <err.h>
#include <SDL/SDL.h>
#include <GL/gl.h>

static int window_width = 640;
static int window_height = 480;

#define HOUR_HAND_SCALE 7.5f
#define MINUTE_HAND_SCALE 10.0f

static const float hand[3][2] = {
	{-0.5f, 0.0f},
	{ 0.5f, 0.0f},
	{ 0.5f, 1.0f}
};

// hour and minute hand transformation matrices;
static float hh_mat[16];
static float mh_mat[16];

void exit_func() {
	SDL_Quit();
}

void initGL() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
		errx(EXIT_FAILURE, "Unable to init SDL: %s\n", SDL_GetError());

	atexit(exit_func);
}

void update_hand_rotation(float* hour_hand_transformation,
                          float* minute_hand_transformation) {
	float rad;
	struct tm *tm;
	time_t t;
	time(&t);
	tm = localtime(&t);

	glLoadIdentity();

	rad = (2.0*M_PI) * tm->tm_hour/24.0 + tm->tm_min/(24.0*60.0);
	glRotatef(rad, 0.0f, 0.0f, -1.0f);
	glTranslatef(0.0f, -0.5f, 0.0f);
	glScalef(1.0f, HOUR_HAND_SCALE, 0.0f);

	glGetFloatv(GL_MODELVIEW_MATRIX, hour_hand_transformation);

	glLoadIdentity();

	rad = (2.0*M_PI) * tm->tm_min/60.0;
	glRotatef(rad, 0.0f, 0.0f, -1.0f);
	glTranslatef(0.0f, -0.5f, 0.0f);
	glScalef(1.0f, MINUTE_HAND_SCALE, 0.0f);

	glGetFloatv(GL_MODELVIEW_MATRIX, minute_hand_transformation);
}

void resize(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void display() {
	size_t i;
	float *mats[2];
	mats[0] = hh_mat;
	mats[1] = mh_mat;

	for (i = 0; i < 2; i++) {
		glLoadMatrixf(mats[i]);
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.5f, 0.0f);
		glVertex2f( 0.5f, 0.0f);
		glVertex2f( 0.0f, 1.0f);
		glEnd();
	}
}

void handle_events() {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			exit(EXIT_SUCCESS);
			break;
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
				window_width = e.window.data1;
				window_height = e.window.data2;
				resize(window_width, window_height);
				DEBUG("New window size: %dx%d\n", window_width, window_height);
			}
			break;
		}
	}
}

int main(int argc, char** argv) {

	SDL_Window* window;
	int window_flags;

	initGL();
	window_flags = SDL_WINDOW_OPENGL;
	window_flags |= SDL_WINDOW_BORDERLESS;
	window_flags |= SDL_WINDOW_RESIZABLE;
	// window_flags |= SDL_WINDOW_FULLSCREEN;
	// window_flags |= SDL_WINDOW_MAXIMIZED;
	window = SDL_CreateWindow("OpenGL Clock",
	                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          window_width, window_height, window_flags);

	resize(window_width, window_height);

	while (true) {
		handle_events();
		update_hand_rotation(hh_mat, mh_mat);
		display();
		SDL_Delay(5);
	}

	return 0;
}

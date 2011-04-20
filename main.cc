#include <ctime>
#include <stdlib.h>
#include <err.h>
#include <SDL/SDL.h>
#include <GL/gl.h>

#define eprintf(fmt, ...) \
	fprintf(stderr, fmt, ##__VA_ARGS__)
#define DEBUG eprintf

#define MAX(a, b) ((a) < (b) ? (b) : (a))

static int window_width = 320;
static int window_height = 320;

#define HOUR_HAND_SCALE 5.0f
#define MINUTE_HAND_SCALE 10.0f

static const float hand[3][2] = {
	{-0.5f, 0.0f},
	{ 0.5f, 0.0f},
	{ 0.5f, 1.0f}
};

// Hour and minute hand transformation matrices.
static float hh_mat[16];
static float mh_mat[16];

void exit_func() {
	fputs("Exiting...\n", stderr);
	SDL_Quit();
}

void initGL() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
		errx(EXIT_FAILURE, "Unable to init SDL: %s\n", SDL_GetError());

	if (
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2) ||
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) ||
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8)              ||
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8)            ||
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8)             ||
		// SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1)    ||
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1)    ||
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4)    ||
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)          ||
		false
		)
	{
		errx(EXIT_FAILURE, "Couldn't set GL attributes: %s\n", SDL_GetError());
	}

	atexit(exit_func);
}

void update_hand_rotation(float* hour_hand_transformation,
                          float* minute_hand_transformation) {
	float deg;
	float scale_inv;
	struct tm *tm;
	time_t t;
	time(&t);
	tm = localtime(&t);

	scale_inv = 1.0f/MAX(HOUR_HAND_SCALE, MINUTE_HAND_SCALE);

	deg = 360.0 * (tm->tm_hour/12.0 + tm->tm_min/(12.0*60.0));

	glLoadIdentity();
	glRotatef(deg, 0.0f, 0.0f, -1.0f);
	glScalef(scale_inv, scale_inv, scale_inv);
	glTranslatef(0.0f, -0.5f, 0.0f);
	glScalef(1.25f, HOUR_HAND_SCALE, 0.0f);

	glGetFloatv(GL_MODELVIEW_MATRIX, hour_hand_transformation);

	deg = 360.0 * (tm->tm_min/60.0 + tm->tm_sec/3600.0);

	glLoadIdentity();
	glRotatef(deg, 0.0f, 0.0f, -1.0f);
	glScalef(scale_inv, scale_inv, scale_inv);
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

void draw() {
	size_t i;
	float *mats[2];
	mats[0] = hh_mat;
	mats[1] = mh_mat;

	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(0.0f, 0.0f, 0.0f);
	for (i = 0; i < 2; i++) {
		glLoadMatrixf(mats[i]);
		glBegin(GL_QUADS);
		glVertex2f(-0.1f, 0.0f);
		glVertex2f( 0.1f, 0.0f);
		glVertex2f( 0.1f, 1.0f);
		glVertex2f(-0.1f, 1.0f);
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
	SDL_GLContext context;

	initGL();

	// Create window.
	{
		Uint32 window_flags;
		window_flags = SDL_WINDOW_OPENGL;
		// window_flags |= SDL_WINDOW_BORDERLESS;
		window_flags |= SDL_WINDOW_RESIZABLE;
		window = SDL_CreateWindow("OpenGL Clock",
		                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		                          window_width, window_height, window_flags);

		if (!window) {
			errx(EXIT_FAILURE, "Couldn't create OpenGL window: %s",
			     SDL_GetError());
		}
	}

	// Create OpenGL context.
	context = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(1);

	resize(window_width, window_height);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	while (true) {
		handle_events();
		update_hand_rotation(hh_mat, mh_mat);
		draw();
		SDL_GL_SwapWindow(window);
		SDL_Delay(50);
	}

	return 0;
}

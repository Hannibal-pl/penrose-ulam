#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <complex.h>
#include <math.h>
#include <cairo/cairo.h>

#define INTEGERS 250000000
#define NROUND 11.0
#define SIZE 500
#define SHOW_NR true

struct TRIANGLE {
	int color;
	double complex A;
	double complex B;
	double complex C;
};

bool integers[INTEGERS];
double pi = 4 * atan(1.0);
double goldenratio = (1.0 + sqrt(5.0)) / 2.0;
double nround = pow(10.0, NROUND);
struct TRIANGLE *triangles = NULL;
int tcount = 0;

static int cmpt(const void *p1, const void *p2) {
	struct TRIANGLE *t1 = (struct TRIANGLE*)p1;
	struct TRIANGLE *t2 = (struct TRIANGLE*)p2;

	double complex c1 = (t1->B + t1->C) * 0.5;
	double complex c2 = (t2->B + t2->C) * 0.5;

	double a1 = round(nround * cabs(c1));
	double a2 = round(nround * cabs(c2));

	double r1 = round(nround * carg(c1));
	double r2 = round(nround * carg(c2));

	if ((a1 < a2) || ((a1 == a2) && (r1 < r2))) {
		return -1;
	} else if ((a1 > a2) || ((a1 == a2) && (r1 > r2))) {
		return 1;
	} else {
		return 0;
	}
}

void eratostenes(void) {
	fprintf(stderr, "Finging primes to %i: ", INTEGERS);

	for(int i = 0; i < INTEGERS; i++) {
		integers[i] = true;
	}
	integers[0] = integers[1] = false;

	for(int i = 0; i < INTEGERS; i++) {
		if (integers[i]) {
			for(int j = i + i; j < INTEGERS; j += i) {
				integers[j] = false;
			}
		}
	}

	fprintf(stderr, "DONE\n");
}

void initTriangles(void) {
	fprintf(stderr, "Init triangles: ");

	tcount = 10;
	triangles = (struct TRIANGLE*) malloc(sizeof(struct TRIANGLE) * tcount);

	for(int i = 0; i < tcount; i++) {
		triangles[i].color = 0;
		triangles[i].A = 0;
		if ((i % 2) == 1) {
			triangles[i].B = cexp(I * (2 * i - 1) * pi / (double) tcount);
			triangles[i].C = cexp(I * (2 * i + 1) * pi / (double) tcount);
		} else {
			triangles[i].B = cexp(I * (2 * i + 1) * pi / (double) tcount);
			triangles[i].C = cexp(I * (2 * i - 1) * pi / (double) tcount);
		}
	}

	fprintf(stderr, "DONE\n");
}

void subdivideTriangles(int times) {
	int ntcount = 0;
	struct TRIANGLE *ntriangles = NULL;
	double complex P;
	double complex Q;
	double complex R;

	for (int i = 1; i <= times; i++) {
		fprintf(stderr, "Subdivide triangles step %i {\n", i);

		fprintf(stderr, "\tCounting colors: ");
		ntcount = 0;
		for(int j = 0; j < tcount; j++) {
			ntcount += (triangles[j].color == 0) ? 2 : 3;
		}
		fprintf(stderr, "DONE\n");

		fprintf(stderr, "\tSpliting: ");
		ntriangles = (struct TRIANGLE*) malloc(sizeof(struct TRIANGLE) * ntcount);
		for(int j = 0, k = 0; j < tcount; j++) {
			if (triangles[j].color == 0) {
				P = triangles[j].A + (triangles[j].B - triangles[j].A) / goldenratio;

				ntriangles[k++] = (struct TRIANGLE) {0, triangles[j].C, P, triangles[j].B};
				ntriangles[k++] = (struct TRIANGLE) {1, P, triangles[j].C, triangles[j].A};
			} else {
				Q = triangles[j].B + (triangles[j].A - triangles[j].B) / goldenratio;
				R = triangles[j].B + (triangles[j].C - triangles[j].B) / goldenratio;

				ntriangles[k++] = (struct TRIANGLE) {1, R, triangles[j].C, triangles[j].A};
				ntriangles[k++] = (struct TRIANGLE) {1, Q, R, triangles[j].B};
				ntriangles[k++] = (struct TRIANGLE) {0, R, Q, triangles[j].A};
			}
		}
		fprintf(stderr, "DONE\n");

		fprintf(stderr, "\tReplacing: ");
		free(triangles);
		triangles = ntriangles;
		tcount = ntcount;
		fprintf(stderr, "DONE\n");

		fprintf(stderr, "} DONE - %i triangles\n\n", tcount);
	}

	fprintf(stderr, "Sorting: ");
	qsort(triangles, tcount, sizeof(struct TRIANGLE), cmpt);
	fprintf(stderr, "DONE\n");
}

void cairo(int nr) {
	char nbuffer[256];
	fprintf(stderr, "Creating output: ");
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, SIZE, SIZE);
	cairo_t *cr = cairo_create(surface);

	cairo_translate(cr, SIZE * 0.5, SIZE * 0.5);
	double wheelRadius = 1.2 * sqrt(SIZE * SIZE * 0.5);
	cairo_scale(cr, wheelRadius, wheelRadius);

	cairo_move_to(cr, -SIZE * 0.5, -SIZE * 0.5);
	cairo_line_to(cr,  SIZE * 0.5, -SIZE * 0.5);
	cairo_line_to(cr,  SIZE * 0.5,  SIZE * 0.5);
	cairo_line_to(cr, -SIZE * 0.5,  SIZE * 0.5);
	cairo_line_to(cr, -SIZE * 0.5, -SIZE * 0.5);
	cairo_close_path(cr);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_fill(cr);

	cairo_set_line_width(cr, cabs(triangles[0].B - triangles[0].A) / 20.0);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_font_size(cr, cabs(triangles[0].B - triangles[0].A) / 5.0);
	cairo_set_source_rgb(cr, 0, 0, 0);

	for(int i = 0; i < tcount; i++) {
		if (integers[(i / 2) + 1]) {
			cairo_set_source_rgb(cr, 0.5, 0, 0);
			cairo_move_to(cr, creal(triangles[i].A), cimag(triangles[i].A));
			cairo_line_to(cr, creal(triangles[i].B), cimag(triangles[i].B));
			cairo_line_to(cr, creal(triangles[i].C), cimag(triangles[i].C));
			cairo_close_path(cr);
			cairo_fill(cr);
			if ((SHOW_NR) && (i % 2)){
				double complex D = (triangles[i].B + triangles[i].C) * 0.5;
				cairo_move_to(cr, creal(D), cimag(D));
				snprintf(nbuffer, sizeof(nbuffer) - 1, "%i", (i / 2) + 1);
				cairo_set_source_rgb(cr, 0, 0, 0);
				cairo_show_text(cr, nbuffer);
			}
		}
		else {
			cairo_set_source_rgb(cr, 0, 0.5, 0);
			cairo_move_to(cr, creal(triangles[i].A), cimag(triangles[i].A));
			cairo_line_to(cr, creal(triangles[i].B), cimag(triangles[i].B));
			cairo_line_to(cr, creal(triangles[i].C), cimag(triangles[i].C));
			cairo_close_path(cr);
			cairo_fill(cr);
			if ((SHOW_NR) && (i % 2)){
				double complex D = (triangles[i].B + triangles[i].C) * 0.5;
				cairo_move_to(cr, creal(D), cimag(D));
				snprintf(nbuffer, sizeof(nbuffer) - 1, "%i", (i / 2) + 1);
				cairo_set_source_rgb(cr, 0, 0, 0);
				cairo_show_text(cr, nbuffer);
			}
		}

		cairo_move_to(cr, creal(triangles[i].C), cimag(triangles[i].C));
		cairo_line_to(cr, creal(triangles[i].A), cimag(triangles[i].A));
		cairo_line_to(cr, creal(triangles[i].B), cimag(triangles[i].B));
		cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
		cairo_stroke(cr);

	}
	fprintf(stderr, "DONE\n");

	fprintf(stderr, "Saving output: ");
	snprintf(nbuffer, sizeof(nbuffer) - 1, "penrose-ulam%i.png", nr);
	cairo_surface_write_to_png (surface, nbuffer);
	cairo_surface_destroy(surface);
	fprintf(stderr, "DONE\n");
}

int main(int argc, char *argv[]) {
	int subdivides = 10;
	if (argc > 1) {
		subdivides = atoi(argv[1]);
		if ((subdivides < 0) || (subdivides > 20)) {
			fprintf(stderr, "Wrong subdivide\n");
			return 1;
		}
		fprintf(stderr, "Subdivides: %i\n", subdivides);
	} else {
		printf("Usage: %s [subdivides]\n", argv[0]);
		return 0;
	}
	eratostenes();
	initTriangles();
	subdivideTriangles(subdivides);
	cairo(subdivides);
	free(triangles);
	return 0;
}

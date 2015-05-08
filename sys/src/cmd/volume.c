#include <u.h>
#include <libc.h>
#include <stdio.h>
#include <draw.h>
#include <event.h>

int volume;
Image *knob;
Image *back;
Image *rim;

Point
volumept(Point c, int volume, int r) {
	double rad = (double) (volume*3.0+30) * PI/180.0;

	c.x -= sin(rad) * r;
	c.y += cos(rad) * r;

	return c;
}

void
redraw(Image *screen)
{
	Point c = divpt(addpt(screen->r.min, screen->r.max), 2);

	draw(screen, screen->r, back, nil, ZP);
	
	line(screen, volumept(c, 0, 45), volumept(c, 0, 40), 0, 0, 1, display->black, ZP);
	line(screen, volumept(c, 100, 45), volumept(c, 100, 40), 0, 0, 1, display->black, ZP);
	ellipse(screen, c, 40, 40, 1, rim, ZP);
	fillellipse(screen, volumept(c, volume, 30), 3, 3, knob, ZP);

	flushimage(display, 1);
}

void
eresized(int new)
{
	if(new && getwindow(display, Refnone) < 0)
		fprint(2,"can't reattach to window");

	redraw(screen);
}

void
main()
{
	int key;
	Mouse m;
	Event e;
	FILE *f;
	char buf[256];
	char *ptr;
	Point p;
	double rad;
	int d;

	f = fopen("/dev/volume", "r");
	if (f == NULL)
		sysfatal ("open volume failed");

	fgets(buf, sizeof(buf), f);
	strtok(buf, " ");
	ptr = strtok(NULL, " ");
	volume = atoi(ptr);

	fclose (f);

	if (initdraw(0, 0, "volume") < 0)
		sysfatal ("initdraw failed");

	einit (Emouse);

	back = allocimagemix (display, 0x88FF88FF, DWhite);
	knob = allocimage (display, Rect(0,0,1,1), CMAP8, 1, 0x008800FF);
	rim = allocimage (display, Rect(0,0,1,1), CMAP8, 1, 0x004400FF);

	redraw(screen);

	for (;;) {
		key = event(&e);

		if(key == Emouse) {
			m = e.mouse;
			if(m.buttons & 1) {
				p = subpt(m.xy, divpt(addpt(screen->r.min, screen->r.max), 2));
				rad = atan2(-p.x, p.y);
				d = rad * 180.0/PI;
				if (d < 0)
					d += 360;

				d *= 160.0/360.0;

				if (d < 30)
					d = 0;
				else if (d > 130)
					d = 100;
				else
					d -= 30;

				volume = d;

				f = fopen("/dev/volume", "w");
				if (f == nil)
					sysfatal ("open volume failed");

				fprintf (f, "%d\n", volume);

				fclose (f);

				redraw(screen);

				sleep(50);
			}
		}
	}
}
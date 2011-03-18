#include "client.h"

int infoTimeCache, infoMessages, infoLastSecond;

//"About" toolbar button handler
void onAboutClicked(GtkWidget *widget, gpointer window) {
	GtkWidget *dialog = gtk_about_dialog_new();
	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog), "Client Viewer");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "0.1");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(c) Team 2");
	gtk_about_dialog_set_comments(
			GTK_ABOUT_DIALOG(dialog),
			"Client Viewer is a program to create a real-time visual representation of the a single robot as generated by the client.");
	const gchar
			*authors[2] = {
					"Peter Neufeld, Frank Lau, Egor Philippov,\nYouyou Yang, Jianfeng Hu, Roy Chiang,\nWilson Huynh, Gordon Leung, Kevin Fahy,\nBenjamin Saunders",
					NULL };

	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

//window destruction methods for the info
void infoDestroy(GtkWidget *window, gpointer widget) {
	gtk_widget_hide_all(GTK_WIDGET(window));
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(widget), FALSE);
}

gboolean infoDeleteEvent(GtkWidget *window, GdkEvent *event, gpointer widget) {
	infoDestroy(window, widget);

	return TRUE;
}

//quit button handler
void quitButtonDestroy(GtkWidget *window, gpointer data) {
	passToQuit* passed = (passToQuit*) data;
	gtk_widget_hide_all(GTK_WIDGET(passed->mainWindow));
	*(passed->viewedRobot) = -1;
}

//window destruction methods for the main window
void mainDestroy(GtkWidget *window, gpointer data) {
	gtk_widget_hide_all(window);
	*((int*) data) = -1;
}

gboolean mainDeleteEvent(GtkWidget *window, GdkEvent *event, gpointer data) {
	mainDestroy(window, data);

	return FALSE;
}

//"Info" toolbar button handler
void onWindowToggled(GtkWidget *widget, gpointer window) {
	GdkColor bgColor;
	gdk_color_parse("black", &bgColor);
	gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &bgColor);

	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)))
		gtk_widget_show_all(GTK_WIDGET(window));
	else
		gtk_widget_hide_all(GTK_WIDGET(window));
}

//called when the spin button value is changed
void onRobotIdChanged(GtkWidget *widget, gpointer data) {
	int *viewedRobot = (int*) data;
	int changedRobotId = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));

	if (*viewedRobot != changedRobotId)
		*viewedRobot = changedRobotId;

	infoTimeCache = 0;
	infoMessages = 0;
	infoLastSecond = 0;
}

//update the drawing of the robot
void ClientViewer::updateViewer(OwnRobot* ownRobot) {
#ifdef DEBUG
	debug << "Drawing robot: " << viewedRobot/*<<", moving at angle of: "<<ownRobot->angle*/<< endl;
	for (unsigned int i = 0; i < ownRobot->seenRobots.size(); i++) {
		debug << "See robot: " << ownRobot->seenRobots.at(i)->id <<" relatively at (" << ownRobot->seenRobots.at(i)->relx << ", "
		<< ownRobot->seenRobots.at(i)->rely << " )" << endl;
	}

	for (unsigned int i = 0; i < ownRobot->seenPucks.size(); i++) {
		debug << "See puck relatively at ( " << ownRobot->seenPucks.at(i)->relx << ", "
		<< ownRobot->seenPucks.at(i)->rely << " )" << endl;
	}

	debug << endl;
#endif

	ownRobotDraw = *ownRobot;
	draw = true;
	gtk_widget_queue_draw(GTK_WIDGET(drawingArea));
}

//update the info window
void updateInfoWindow(OwnRobot* ownRobotDraw, GtkBuilder* builder) {
	infoTimeCache = time(NULL);
	infoMessages++;

	if (infoTimeCache > infoLastSecond) {
		string tmp;

		tmp = "Traveling with a velocity of " + helper::toString(ownRobotDraw->vx) + ", " + helper::toString(
				ownRobotDraw->vy) + "( x, y )";
		gtk_label_set_text(GTK_LABEL(gtk_builder_get_object( builder, "labSpeed" )), tmp.c_str());

		tmp = "Located at " + helper::toString(ownRobotDraw->homeRelX) + ", "
				+ helper::toString(ownRobotDraw->homeRelY) + " (x,y) relative to its home";
		gtk_label_set_text(GTK_LABEL(gtk_builder_get_object( builder, "labHome" )), tmp.c_str());

		tmp = "Traveling with at an angle of " + helper::toString(ownRobotDraw->angle);
		gtk_label_set_text(GTK_LABEL(gtk_builder_get_object( builder, "labAngle" )), tmp.c_str());

		if (ownRobotDraw->hasPuck)
			tmp = "Carrying a puck";
		else
			tmp = "NOT carrying a puck";

		if (ownRobotDraw->hasCollided)
			tmp += " and colliding";
		else
			tmp += " and NOT colliding";
		gtk_label_set_text(GTK_LABEL(gtk_builder_get_object( builder, "labPuck" )), tmp.c_str());

		tmp = helper::toString(infoMessages) + " updates per second";
		gtk_label_set_text(GTK_LABEL(gtk_builder_get_object( builder, "labFrames" )), tmp.c_str());

		infoMessages = 0;
		infoLastSecond = infoTimeCache;
	}
}

const float arrowLength=0.15, arrowDegrees=0.35;
//calculate arrow vertexes
//taken from: http://kapo-cpp.blogspot.com/2008/10/drawing-arrows-with-cairo.html
void calcVertexes(int start_x, int start_y, int end_x, int end_y, int drawFactor, double& x1, double& y1, double& x2,
		double& y2) {
	double angle = atan2(end_y - start_y, end_x - start_x) + M_PI;

	x1 = end_x + arrowLength*drawFactor * cos(angle - arrowDegrees);
	y1 = end_y + arrowLength*drawFactor * sin(angle - arrowDegrees);
	x2 = end_x + arrowLength*drawFactor * cos(angle + arrowDegrees);
	y2 = end_y + arrowLength*drawFactor * sin(angle + arrowDegrees);
}


//called on the drawingArea's expose event
gboolean drawingAreaExpose(GtkWidget *widgetDrawingArea, GdkEventExpose *event, gpointer data) {
	passToDrawingAreaExpose* passed = (passToDrawingAreaExpose*) data;
	bool* draw = passed->draw;

	if (*draw) {
		OwnRobot* ownRobotDraw = passed->ownRobotDraw;
		int myTeam = passed->myTeam;
		int numberOfRobots = passed->numberOfRobots;
		int *drawFactor = passed->drawFactor;
		int imageWidth = (VIEWDISTANCE * 2 + ROBOTDIAMETER) * (*drawFactor) / 4;
		int imageHeight = (VIEWDISTANCE * 2 + ROBOTDIAMETER) * (*drawFactor) / 4;

		//origin is the middle point where our viewed robot is located
		int origin[] = { imageWidth / 2, imageHeight / 2 };

		cairo_t *cr = gdk_cairo_create(GTK_DRAWING_AREA(widgetDrawingArea)->widget.window);
		cairo_set_line_width(cr, 2);

		ColorObject color = colorFromTeam(myTeam);

		//draw our home if it is nearby (FIRST so it goes underneath)
		if (ownRobotDraw->homeRelX - ((HOMEDIAMETER / 2) + (ROBOTDIAMETER / 2)) < VIEWDISTANCE &&
				ownRobotDraw->homeRelY - ((HOMEDIAMETER / 2) + (ROBOTDIAMETER / 2)) < VIEWDISTANCE) {
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_arc(cr, origin[0] + ownRobotDraw->homeRelX * *drawFactor,
					origin[1] + ownRobotDraw->homeRelY * *drawFactor, HOMEDIAMETER * *drawFactor / 8, 0, 2 * M_PI);
			cairo_stroke_preserve(cr);
			cairo_set_source_rgb(cr, color.r, color.g, color.b);
			cairo_fill(cr);
		}

		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_arc(cr, origin[0], origin[1], ROBOTDIAMETER * *drawFactor / 8, 0, 2 * M_PI);
		cairo_stroke_preserve(cr);
		cairo_set_source_rgb(cr, color.r, color.g, color.b);
		cairo_fill(cr);

		//if the robot has a puck then actually draw the puck in its center
		//WARNING: if a robot has a puck, does it still see the puck?
		if (ownRobotDraw->hasPuck) {
			cairo_set_source_rgb(cr, 0, 0, 0);

			cairo_arc(cr, origin[0], origin[1], PUCKDIAMETER * *drawFactor / 6, 0, 2 * M_PI);
			cairo_stroke(cr);
		}

		//ownRobot->angle does not provide a valid angle ( it's always zero )!
		//draw the line showing the angle that the robot is moving at
		if( !(ownRobotDraw->vx == 0 && ownRobotDraw->vy == 0 ))
		{
			int endX=origin[0] + ownRobotDraw->vx * *drawFactor, endY=origin[1] + ownRobotDraw->vy * *drawFactor;

			double x1, y1, x2, y2;
			calcVertexes(origin[0], origin[1], endX, endY, *drawFactor, x1, y1, x2, y2);

			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_move_to(cr, origin[0], origin[1]);
			cairo_line_to(cr, endX, endY);
			cairo_line_to(cr, x1, y1);
			cairo_move_to(cr, x2, y2);
			cairo_line_to(cr, endX, endY);

			cairo_stroke(cr);
		}

		for (unsigned int i = 0; i < ownRobotDraw->seenRobots.size(); i++) {
			color = colorFromTeam((ownRobotDraw->seenRobots.at(i)->id) / numberOfRobots);
			cairo_set_source_rgb(cr, 0, 0, 0);
			cairo_arc(cr, origin[0] + ownRobotDraw->seenRobots.at(i)->relx * *drawFactor,
					origin[1] + ownRobotDraw->seenRobots.at(i)->rely * *drawFactor, ROBOTDIAMETER * *drawFactor / 8, 0,
					2 * M_PI);
			cairo_stroke_preserve(cr);

			cairo_set_source_rgb(cr, color.r, color.g, color.b);
			cairo_fill(cr);
		}

		//draw seen pucks
		cairo_set_source_rgb(cr, 0, 0, 0);
		for (unsigned int i = 0; i < ownRobotDraw->seenPucks.size(); i++) {
			cairo_arc(cr, origin[0] + ownRobotDraw->seenPucks.at(i)->relx * *drawFactor,
					origin[1] + ownRobotDraw->seenPucks.at(i)->rely * *drawFactor, PUCKDIAMETER * *drawFactor / 8, 0,
					2 * M_PI);
			cairo_fill(cr);
		}

		//draw a rectangle around the drawing area
		cairo_rectangle(cr, 0, 0, imageWidth, imageHeight);
		cairo_stroke(cr);

		cairo_destroy(cr);
		*draw = false;

		if (gtk_toggle_tool_button_get_active(passed->info))
			updateInfoWindow(ownRobotDraw, passed->builder);
	}

	return FALSE;
}

//resize the drawing area based on the drawFactor
void resizeByDrawFactor(int drawFactor, GtkDrawingArea *drawingArea, GtkWidget *mainWindow) {
	//drawing related variables
	int imageWidth = (VIEWDISTANCE * 2 + ROBOTDIAMETER) * (drawFactor) / 4;
	int imageHeight = (VIEWDISTANCE * 2 + ROBOTDIAMETER) * (drawFactor) / 4;

#ifdef DEBUG
	cout << "imageWidth is " << imageWidth << " and the imageHeight is " << imageHeight << endl;
#endif

	gtk_widget_set_size_request(GTK_WIDGET(drawingArea), imageWidth, imageHeight);
	gtk_window_resize(GTK_WINDOW(mainWindow), imageWidth, imageHeight);
}

//zoom in button handler
void onZoomInClicked(GtkWidget *widgetDrawingArea, gpointer data) {

	passToZoom *passed = (passToZoom*) data;
	int *drawFactor = passed->drawFactor;

	if (*drawFactor >= MAXZOOMED) {
		gtk_widget_set_sensitive(GTK_WIDGET(passed->zoomIn), false);
	} else {
		GtkWidget *zoomOut = GTK_WIDGET(passed->zoomOut);
		*drawFactor = *drawFactor + ZOOMSPEED;

		resizeByDrawFactor(*drawFactor, passed->drawingArea, passed->mainWindow);

		if (!gtk_widget_get_sensitive(zoomOut) && *drawFactor <= MAXZOOMED)
			gtk_widget_set_sensitive(zoomOut, true);
	}
}

//zoom out button hundler
void onZoomOutClicked(GtkWidget *widgetDrawingArea, gpointer data) {

	passToZoom *passed = (passToZoom*) data;
	int *drawFactor = passed->drawFactor;

	if (*drawFactor <= MINZOOMED) {
		gtk_widget_set_sensitive(GTK_WIDGET(passed->zoomOut), false);
	} else {
		GtkWidget *zoomIn = GTK_WIDGET(passed->zoomIn);
		*drawFactor = *drawFactor - ZOOMSPEED;

		resizeByDrawFactor(*drawFactor, passed->drawingArea, passed->mainWindow);

		if (!gtk_widget_get_sensitive(zoomIn) && *drawFactor <= MAXZOOMED)
			gtk_widget_set_sensitive(zoomIn, true);
	}
}

//initializations and simple modifications for the things that will be drawn
void ClientViewer::initClientViewer(int numberOfRobots, int myTeam, int _drawFactor) {
#ifdef DEBUG
	debug << "Starting the Client Viewer!" << endl;
#endif
	g_type_init();

	//load the builder file
	gtk_builder_add_from_file(builder, builderPath.c_str(), NULL);

	GtkWidget *mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	GtkToggleToolButton *info = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(builder, "Info"));
	GtkToolButton *zoomIn = GTK_TOOL_BUTTON(gtk_builder_get_object(builder, "ZoomIn"));
	GtkToolButton *zoomOut = GTK_TOOL_BUTTON(gtk_builder_get_object(builder, "ZoomOut"));
	GtkWidget *about = GTK_WIDGET(gtk_builder_get_object(builder, "About"));
	GtkWidget *infoWindow = GTK_WIDGET(gtk_builder_get_object(builder, "infoWindow"));
	GtkSpinButton *robotId = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "robotId"));
	GdkColor color;

	//drawing related variables
	drawFactor = new int();
	*drawFactor = _drawFactor;
	int imageWidth = (VIEWDISTANCE * 2 + ROBOTDIAMETER) * (*drawFactor) / 4, imageHeight = (VIEWDISTANCE * 2
			+ ROBOTDIAMETER) * (*drawFactor) / 4;

#ifdef DEBUG
	debug << "imageWidth is " << imageWidth << " and the imageHeight is " << imageHeight << endl;
#endif
	drawingArea = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "drawingArea"));

	gtk_widget_set_size_request(GTK_WIDGET(drawingArea), imageWidth, imageHeight);
	gtk_window_resize(GTK_WINDOW(mainWindow), imageWidth, imageHeight);

	//set the upper value for the spin button
	gtk_adjustment_set_upper(GTK_ADJUSTMENT(gtk_builder_get_object(builder, "robotIdAdjustment")), numberOfRobots - 1);
	gtk_adjustment_set_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder, "robotIdAdjustment")), -1);

	//keep the info window floating on top of the main window
	gtk_window_set_keep_above(GTK_WINDOW(infoWindow), true);

	//change the color of the main window's background to black
	gdk_color_parse("black", &color);
	gtk_widget_modify_bg(GTK_WIDGET(mainWindow), GTK_STATE_NORMAL, &color);

	//change the color of the info window text
	gdk_color_parse("white", &color);
	gtk_widget_modify_fg(GTK_WIDGET(gtk_builder_get_object(builder, "labSpeed")), GTK_STATE_NORMAL, &color);
	gtk_widget_modify_fg(GTK_WIDGET(gtk_builder_get_object(builder, "labAngle")), GTK_STATE_NORMAL, &color);
	gtk_widget_modify_fg(GTK_WIDGET(gtk_builder_get_object(builder, "labFrames")), GTK_STATE_NORMAL, &color);
	gtk_widget_modify_fg(GTK_WIDGET(gtk_builder_get_object(builder, "labPuck")), GTK_STATE_NORMAL, &color);
	gtk_widget_modify_fg(GTK_WIDGET(gtk_builder_get_object(builder, "labHome")), GTK_STATE_NORMAL, &color);
	gtk_widget_modify_bg(GTK_WIDGET(drawingArea), GTK_STATE_NORMAL, &color);

	g_signal_connect(robotId, "value-changed", G_CALLBACK(onRobotIdChanged), (gpointer) & viewedRobot);

	g_signal_connect(info, "toggled", G_CALLBACK(onWindowToggled), (gpointer) infoWindow);
	g_signal_connect(about, "clicked", G_CALLBACK(onAboutClicked), (gpointer) mainWindow);

	g_signal_connect(zoomIn, "clicked", G_CALLBACK(onZoomInClicked), (gpointer)(new passToZoom( drawFactor, mainWindow, drawingArea, zoomIn, zoomOut)));
	g_signal_connect(zoomOut, "clicked", G_CALLBACK(onZoomOutClicked), (gpointer)(new passToZoom(drawFactor, mainWindow, drawingArea, zoomIn, zoomOut)));

	g_signal_connect(gtk_builder_get_object(builder, "Exit"), "clicked", G_CALLBACK(quitButtonDestroy), (gpointer)new passToQuit(&viewedRobot, mainWindow));

	g_signal_connect(mainWindow, "destroy", G_CALLBACK(mainDestroy), (gpointer) &viewedRobot);
	g_signal_connect(mainWindow, "delete-event", G_CALLBACK(mainDeleteEvent), (gpointer) &viewedRobot);

	g_signal_connect(infoWindow, "destroy", G_CALLBACK(infoDestroy), (gpointer) info);
	g_signal_connect(infoWindow, "delete-event", G_CALLBACK(infoDeleteEvent), (gpointer) info);

	g_signal_connect(drawingArea, "expose-event", G_CALLBACK(drawingAreaExpose), (gpointer)(
					new passToDrawingAreaExpose(myTeam, numberOfRobots, drawFactor, &draw, &ownRobotDraw, info, builder)));

	gtk_widget_show_all(mainWindow);
}

ClientViewer::ClientViewer(string pathToExe) :
	viewedRobot(-1), draw(false) {

	//assume that the clientviewer.builder is in the same directory as the executable that we are running
	builderPath = pathToExe + "clientviewer.glade";
	builder = gtk_builder_new();

#ifdef DEBUG
	debug.open(helper::clientViewerDebugLogName.c_str(), ios::out);
#endif
}

ClientViewer::~ClientViewer() {
	g_object_unref(G_OBJECT(builder));
#ifdef DEBUG
	debug.close();
#endif
}

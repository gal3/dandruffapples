#include "client.h"

//"About" toolbar button handler
void on_About_clicked(GtkWidget *widget, gpointer window) {
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

//window destruction methods for the info and navigation windows
void destroy(GtkWidget *window, gpointer widget) {
	gtk_widget_hide_all(GTK_WIDGET(window));
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(widget), FALSE);
}

gboolean delete_event(GtkWidget *window, GdkEvent *event, gpointer widget) {
	destroy(window, widget);

	return TRUE;
}

//"Robot" and "Info" toolbar button handler
void on_Window_toggled(GtkWidget *widget, gpointer window) {
	GdkColor bgColor;
	gdk_color_parse("black", &bgColor);
	gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &bgColor);

	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)))
		gtk_widget_show_all(GTK_WIDGET(window));
	else
		gtk_widget_hide_all(GTK_WIDGET(window));
}

void on_RobotId_changed(GtkWidget *widget, gpointer _passedData) {
	dataToHandler* passedData = (dataToHandler*) _passedData;
	int *currentRobot = (int*) passedData->data;
	int changedRobotId = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));

	if (*((int*) currentRobot) != changedRobotId) {
#ifdef DEBUG
		*(passedData->debug)<<"currentRobot changed from "<<*((int*)currentRobot)<<" to "<<changedRobotId<<endl;
#endif
		cout << "currentRobot changed from " << *((int*) currentRobot) << " to " << changedRobotId << endl;
		*((int*) currentRobot) = changedRobotId;
	}
}

//update the drawing of the robot
//this is called after the client has written to the async queue
void ClientViewer::updateViewer()
{
	OwnRobot* ownRobot;
	//watch out for the case where popping will return NULL ( should never happen )
	ownRobot = (OwnRobot*)g_async_queue_try_pop(asyncQueue);

#ifdef DEBUG
	debug<<"Read robot at "<<ownRobot->vx<<" and "<<ownRobot->vy<<endl;
#endif
}

//ClientViewer::
//initializations and simple modifications for the things that will be drawn
void ClientViewer::initClientViewer(int numberOfRobots) {
#ifdef DEBUG
	debug<<"Starting the Client Viewer!"<<endl;
#endif
	g_type_init();

	GtkWidget *mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	GtkToggleToolButton *info = GTK_TOGGLE_TOOL_BUTTON(gtk_builder_get_object(builder, "Info"));
	GtkWidget *about = GTK_WIDGET(gtk_builder_get_object(builder, "About"));
	GtkWidget *infoWindow = GTK_WIDGET(gtk_builder_get_object(builder, "infoWindow"));
	GtkSpinButton *robotId = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "robotId"));
	GdkColor color;

	gtk_adjustment_set_upper(GTK_ADJUSTMENT(gtk_builder_get_object(builder, "robotIdAdjustment")), numberOfRobots);

	//keep the info window floating on top of the main window
	gtk_window_set_keep_above(GTK_WINDOW(infoWindow), true);

	//change the color of the main window's background to black
	gdk_color_parse("black", &color);
	gtk_widget_modify_bg(GTK_WIDGET(mainWindow), GTK_STATE_NORMAL, &color);

	//change the color of the label's text to white
	gdk_color_parse("white", &color);
	//gtk_widget_modify_fg(GTK_WIDGET(gtk_builder_get_object(builder, "robotWindowLabel")), GTK_STATE_NORMAL, &color);


	dataToHandler data(&debug, currentRobot);
	g_signal_connect(robotId, "value-changed", G_CALLBACK(on_RobotId_changed), (gpointer) & data);

	g_signal_connect(info, "toggled", G_CALLBACK(on_Window_toggled), (gpointer) infoWindow);
	g_signal_connect(about, "clicked", G_CALLBACK(on_About_clicked), (gpointer) mainWindow);

	g_signal_connect(infoWindow, "destroy", G_CALLBACK(destroy), (gpointer) info);
	g_signal_connect(infoWindow, "delete-event", G_CALLBACK(delete_event), (gpointer) info);

	gtk_widget_show_all(mainWindow);

	gtk_main();
}

ClientViewer::ClientViewer(int argc, char* argv[],  GAsyncQueue* _asyncQueue, int* _currentRobot) :
	currentRobot(_currentRobot), asyncQueue(_asyncQueue) {
	gtk_init(&argc, &argv);

	//assume that the clientviewer.builder is in the same directory as the executable that we are running
	string builderPath(argv[0]);
	builderPath = builderPath.substr(0, builderPath.find_last_of("//") + 1) + "clientviewer.glade";
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, builderPath.c_str(), NULL);
	gtk_builder_connect_signals(builder, NULL);

#ifdef DEBUG
	debug.open(helper::clientViewerDebugLogName.c_str(), ios::out);
#endif
}

ClientViewer::~ClientViewer() {
	g_async_queue_unref(asyncQueue);
	g_object_unref( G_OBJECT(builder));
#ifdef DEBUG
	debug.close();
#endif
}

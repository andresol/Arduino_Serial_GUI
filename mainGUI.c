#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <time.h>
#include <getopt.h>

#include "mainGUI.h"
#include "arduino-serial-lib.h"

//#define DEBUG

static int fd = -1;
char* device = NULL;

void usagePrint() {
	printf("Usage: arduino-serial -b <bps> -p <serialport> [OPTIONS]\n"
			"\n"
			"Options:\n"
			"  -h, --help                 Print this help message\n"
			"  -b, --baud=baudrate        Baudrate (bps) of Arduino (default 9600)\n"
			"  -n, --nongui               Start in console mode\n"
			"  -p, --port=serialport      Serial port Arduino is connected to\n"
			"  -q  --quiet                Don't print out as much info\n"
			"\n"
			"\n");
	exit(EXIT_SUCCESS);
}

gint connectSerial() {
#ifdef DEBUG
	printf("Connecting to device %s\n", DEFAULT_DEVICE);
#endif
	if (fd >= 0) {
		return setupSerial(device);
	}
	return -1;
}

gint disconnectSerial() {
	if (fd >= 0) {
		serialport_close(fd);
	}
	return -1;
}

int update_statusbar(GtkTextBuffer *buffer, GtkStatusbar  *statusbar) {
	gchar *msg;
	gint row, col;
	GtkTextIter iter;

	gtk_statusbar_pop(statusbar, 0);

	gtk_text_buffer_get_iter_at_mark(buffer,
			&iter, gtk_text_buffer_get_insert(buffer));

	row = gtk_text_iter_get_line(&iter);
	col = gtk_text_iter_get_line_offset(&iter);

	msg = g_strdup_printf("Col %d Ln %d", col+1, row+1);

	gtk_statusbar_push(statusbar, 0, msg);

	g_free(msg);
	return 0;
}


static void mark_set_callback(GtkTextBuffer *buffer,
		const GtkTextIter *new_location, GtkTextMark *mark,
		gpointer data) {
	update_statusbar(buffer, GTK_STATUSBAR(data));
}

int main( int argc, char *argv[]){

	/* parse options */
	int option_index = 0, opt;

	int gui = 1;
	static struct option loptions[] = {
			{"help",       no_argument,       0, 'h'},
			{"port",       required_argument, 0, 'p'},
			{"nongui",     required_argument, 0, 'n'},
			{"quiet",      no_argument,       0, 'q'},
			{0,         0,                 0, 0}
	};

	do {
		opt = getopt_long (argc, argv, "hp:nq",
				loptions, &option_index);

		switch (opt) {
		case '0':
			break;
		case 'p':
			device = strdup(optarg);
			break;
		case 'n':
			gui = 0;
			break;
		case 'h':
			usagePrint();
			break;
			//default:
		}
	} while(opt != -1);

	if (device == NULL) {
		device = strdup(DEFAULT_DEVICE);
	}
	if (gui) {
		return doGUI(argc, argv);
	} else {
		return doConsole(argc, argv);
	}
	return 0;
}

int doGUI( int argc, char *argv[]) {
	GtkWidget *window;
	GtkWidget *vbox;

	GtkWidget *toolbar;
	GtkWidget *view;
	GtkWidget* scrolledwindow;
	GtkWidget *statusbar;
	GtkToolItem *exit;
	GtkToolItem *properties;
	GtkToolItem *connect;
	GtkTextBuffer *buffer;
	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_set_title(GTK_WINDOW(window), MAIN_GUI_TITLE);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);

	exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit, -1);

	connect = gtk_tool_button_new_from_stock(GTK_STOCK_CONNECT);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), connect, -1);

	properties = gtk_tool_button_new_from_stock(GTK_STOCK_PROPERTIES);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), properties, -1);

	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 5);

	view = gtk_text_view_new();
	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), view);
	gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, TRUE, TRUE, 0);
	gtk_widget_grab_focus(view);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(exit), "clicked",
			G_CALLBACK(gtk_main_quit), NULL);

	g_signal_connect(G_OBJECT(connect), "clicked",
			G_CALLBACK(connectSerial), NULL);

	g_signal_connect(buffer, "changed",
			G_CALLBACK(update_statusbar), statusbar);

	g_signal_connect_object(buffer, "mark_set",
			G_CALLBACK(mark_set_callback), statusbar, 0);

	g_signal_connect_swapped(G_OBJECT(window), "destroy",
			G_CALLBACK(gtk_main_quit), NULL);

	fd = setupSerial(device);

	g_timeout_add(1000, (GSourceFunc) time_handler, (gpointer) view);

	gtk_widget_show_all(window);

	update_statusbar(buffer, GTK_STATUSBAR (statusbar));

	gtk_main();
}

int doConsole(int argc, char *argv[]) {

}

void errorPrint(char* msg) {
	fprintf(stderr, "%s\n",msg);
}

int setupSerial(char* port) {
	fd = -1;
	fd = serialport_init(port, DEFAULT_BAUDRATE);
	if( fd==-1 ){
		errorPrint("couldn't open port");
		return fd;
	}
	serialport_flush(fd);
	return fd;
}

int readFromSerial(GtkWidget *view) {
	if (fd == -1) {
		gtkTextviewAppend(view, "Error opening device.\n");
		return 0;
	}
	const int buf_max = 256;
	char buf[buf_max];
	char timeText[buf_max*2];
	char eolchar = '\n';
	gchar text[buf_max*2];
	time_t now;
	memset(buf,0,buf_max);
	memset(timeText,0,buf_max*2);
	memset(text,0,buf_max*2);
	serialport_read_until(fd, buf, eolchar, buf_max, 5000);
	now = time(NULL);
	strftime(timeText, sizeof(timeText), "T:%Y-%m-%d %H:%M:%S ", localtime(&now));
	strcat(timeText,buf);
	strcpy(text,timeText);
	if (buf[0] != '\0') {
		gtkTextviewAppend(view, text);
	}
	return 0;
}

void gtkTextviewAppend(GtkWidget *textview, gchar *text) {
	GtkTextBuffer *tbuffer;
	GtkTextIter ei;

	tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_end_iter(tbuffer, &ei);
	gtk_text_buffer_insert(tbuffer, &ei, text, -1);
}

static gboolean time_handler(GtkWidget *widget) {
	if (fd == -1) return FALSE;
	readFromSerial(widget);
	return TRUE;
}

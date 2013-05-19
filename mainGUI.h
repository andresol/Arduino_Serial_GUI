/*
 * mainGUI.h
 *
 *  Created on: May 16, 2013
 *      Author: soland
 */

#ifndef MAINGUI_H_
#define MAINGUI_H_

#include <glib.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define DEFAULT_BAUDRATE 9600
static const char* DEFAULT_DEVICE = "/dev/ttyUSB0";
static const char* MAIN_GUI_TITLE = "Arduino Serial Monitor";

void usagePrint(void);
gint connectSerial(void);
gint disconnectSerial(void);
int setupSerial(char* port);
void errorPrint(char* msg);
int readFromSerial(GtkWidget *view);
void gtkTextviewAppend(GtkWidget *textview, gchar *text);
static gboolean time_handler(GtkWidget *widget);
int doGUI(int argc, char *argv[]);
int doConsole(int argc, char *argv[]);

#endif /* MAINGUI_H_ */

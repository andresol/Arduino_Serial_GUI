/*
 * mainGUI.h
 *
 *  Created on: May 16, 2013
 *      Author: soland
 */

#ifndef MAINGUI_H_
#define MAINGUI_H_

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define DEFAULT_BAUDRATE 9600

int setupSerial(char* port);
void errorPrint(char* msg);
int readFromSerial(GtkWidget *view);
void gtkTextviewAppend(GtkWidget *textview, gchar *text);
static gboolean time_handler(GtkWidget *widget);

#endif /* MAINGUI_H_ */

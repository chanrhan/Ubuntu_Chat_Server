#ifndef GTK_UTILS_H
#define GTK_UTILS_H

#include "socket_common.h"
#include <gtk-3.0/gtk/gtk.h>
#include <gtk-3.0/gtk/gtktypes.h>

int reload_service_page();

GtkWidget* gtk_create_room_item(room_vo* vo);
GtkWidget* gtk_create_chat_item(int index, chat_vo* vo, int is_mine);

int reload_room_list();
void reload_chat_list(int room_id);

void remove_all_children(GtkWidget* container_widget);

void draw_room_list();
void draw_chat_list();

GtkWidget* get_chat_item(int index);

void show_modal_snackbar(char* text);

void hide_modal_snackbar();

void show_modal_chat_context_menu(int top, int left);
void hide_modal_chat_context_menu();
#endif

#ifndef GTK_MODAL_H
#define GTK_MODAL_H

#include "socket_common.h"
#include <gtk-3.0/gtk/gtk.h>
#include <gtk-3.0/gtk/gtktypes.h>

typedef struct user_t{
    char* id;
    char* name;
    struct user_t* next;
}user_t;

typedef struct gtk_modal_add_room{
    GtkWidget* window;
    GtkWidget* inp_room_name;
    GtkWidget* inp_search_user;
    GtkWidget* user_liststore;
    GtkWidget* user_list_box;
    GtkWidget* btn_search_user;
    GtkWidget* btn_cancel;
    GtkWidget* btn_submit;
    user_t* invite_user_list;
}gtk_modal_add_room;

typedef struct gtk_modal_search_user{
    GtkWidget* window;
    GtkWidget* inp_username;
    GtkWidget* btn_search;
    GtkWidget* btn_cancel;
    GtkWidget* btn_submit;
    user_t* invite_user_list;
}gtk_modal_search_user;

typedef struct gtk_modal_snackbar{
    GtkWidget* window;
    GtkWidget* type_label;
    GtkWidget* text;
}gtk_modal_snackbar;

typedef struct gtk_modal_chat_context_menu{
    GtkWidget* window;
    GtkWidget* item_reply;
    GtkWidget* item_react;
    GtkWidget* item_delete;
    int curr_chat_id;
}gtk_modal_chat_context_menu;

typedef struct gtk_modal_react_context_menu{
    GtkWidget* window;
    chat_react_type react_type;
    int curr_chat_id;
}gtk_modal_react_context_menu;

typedef struct gtk_modal_delete_context_menu{
    GtkWidget* window;
    GtkWidget* item_delete_all;
    int curr_chat_id;
}gtk_modal_delete_context_menu;

extern gtk_modal_add_room md_add_room;
extern gtk_modal_search_user md_search_user;
extern gtk_modal_snackbar md_snackbar;
extern gtk_modal_chat_context_menu md_chat_context_menu;

extern gtk_modal_react_context_menu md_react_context_menu;
extern gtk_modal_delete_context_menu md_delete_context_menu;


void init_modal(GtkWidget* parent);

void show_md_react_context_menu(GtkWidget *anchor, int chat_id);
void show_md_emoji_context_menu();

#endif
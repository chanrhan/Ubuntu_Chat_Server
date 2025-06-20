#ifndef GTK_CALLBACK_H
#define GTK_CALLBACK_H

#include <gtk-3.0/gtk/gtk.h>
#include <gtk-3.0/gtk/gtktypes.h>


#define DECLARE_GTK_CALLBACK(str) int g_callback_##str(GtkWidget* widget, gpointer data)


DECLARE_GTK_CALLBACK(login);
DECLARE_GTK_CALLBACK(signup);
DECLARE_GTK_CALLBACK(move_login_page);
DECLARE_GTK_CALLBACK(move_signup_page);

DECLARE_GTK_CALLBACK(open_add_room);

DECLARE_GTK_CALLBACK(close_add_room);
DECLARE_GTK_CALLBACK(add_room);
DECLARE_GTK_CALLBACK(select_room);
DECLARE_GTK_CALLBACK(open_room_menu);
DECLARE_GTK_CALLBACK(remove_list_user_item);

DECLARE_GTK_CALLBACK(search_room);
DECLARE_GTK_CALLBACK(search_user);
DECLARE_GTK_CALLBACK(search_chat);
DECLARE_GTK_CALLBACK(send_chat);

DECLARE_GTK_CALLBACK(chat_reply);
DECLARE_GTK_CALLBACK(delete_chat_self);
DECLARE_GTK_CALLBACK(delete_chat_all);
// DECLARE_GTK_CALLBACK(chat_react);
DECLARE_GTK_CALLBACK(chat_delete);
DECLARE_GTK_CALLBACK(select_react_icon);

DECLARE_GTK_CALLBACK(show_emoji_context_menu);
DECLARE_GTK_CALLBACK(select_emoji);

int g_callback_chat_react(GtkWidget* widget, GdkEventButton *event, gpointer data);

gboolean on_key_press(GtkWidget *widget,GdkEventKey *event, gpointer user_data);

gboolean on_deactivate_delete_context_menu(GtkWidget *widget, GdkEventCrossing *event, gpointer data);
gboolean on_deactivate_chat_context_menu(GtkWidget *widget, GdkEventCrossing *event, gpointer data);

gboolean on_hide_chat_context_menu(GtkWidget *widget, GdkEventCrossing *event, gpointer data);

int g_callback_onclick_chat_item(GtkWidget* widget, GdkEventButton *event, gpointer data);
int g_callback_onclick_delete_item(GtkWidget* widget, GdkEventButton *event, gpointer data);

int on_select_searched_user(GtkEntryCompletion *completion,
                           GtkTreeModel *model,
                           GtkTreeIter *iter,
                           gpointer user_data);

gboolean on_modal_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);

int g_callback_item_clicked(GtkWidget* widget, GdkEventButton *event, gpointer data);

int gtk_exit(GtkWidget* widget, gpointer data);

gboolean on_enter_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);

gboolean on_leave_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);

// gboolean on_enter_notify_path_tok(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);

// gboolean on_leave_notify_path_tok(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);

void on_popup_close(GtkWidget *widget, gpointer user_data);

#endif
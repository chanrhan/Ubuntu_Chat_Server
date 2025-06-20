#ifndef GTK_DRAW_H
#define GTK_DRAW_H

#include <gtk-3.0/gtk/gtk.h>
#include <gtk-3.0/gtk/gtktypes.h>
#include "socket_common.h"

#define PATH_LOVE_ICON "src/resources/images/emo_love.png"
#define PATH_LIKE_ICON "src/resources/images/emo_like.png"
#define PATH_CHECK_ICON "src/resources/images/emo_check.png"
#define PATH_LAUGH_ICON "src/resources/images/emo_laugh.png"
#define PATH_SURPRISE_ICON "src/resources/images/emo_surprise.png"
#define PATH_SAD_ICON "src/resources/images/emo_sad.png"

typedef struct LoginPage{
    GtkWidget* page;
    GtkWidget* id_input;
    GtkWidget* pwd_input;
    GtkWidget* name_input;
}LoginPage;

typedef struct SignupPage{
    GtkWidget* page;
    GtkWidget* id_input;
    GtkWidget* pwd_input;
    GtkWidget* name_input;
}SignupPage;

typedef struct ServicePage{
    GtkWidget* page;
    GtkWidget* room_list_scroll;
    GtkWidget* room_list;
    GtkWidget* inp_search_room;
    GtkWidget* chat_list_scroll;
    GtkWidget* chat_list;
    GtkWidget* inp_search_chat;
    GtkWidget* room_name_box;
    GtkWidget* chat_notice;
    GtkWidget* inp_chat;
    GtkWidget* btn_open_emoji_context;
    GtkWidget* btn_room_add;
    GtkWidget* btn_room_menu;

    GtkWidget* reply_box;
    GtkWidget* reply_user_id_label;
    GtkWidget* reply_text_label;
    int curr_reply_id;

    int curr_room_id;
    // int room_id_list[100];
    // char* room_name_list[100];
    room_vo* room_list_data;
    int room_list_size;

    GtkWidget** chat_item_list;
    chat_vo* chat_list_data;
    int chat_list_size;
}ServicePage;

static chat_react_type react_types[] = {
            LOVE, LIKE, CHECK, LAUGH, SURPRISE, SAD
        };

extern GtkWidget* main_window;
extern GtkWidget* page_stack;

extern LoginPage* login_page;
extern SignupPage* signup_page;
extern ServicePage* service_page;

gboolean on_realize(gpointer data);

void create_gtk_main_window();

void css_init();

void css_reload();

void build_layout();

void gtk_style_class_toggle(GtkWidget* widget, char* classname, gboolean flag);

GtkWidget* gtk_create_vbox(int width, int height, char* classname);
GtkWidget* gtk_create_hbox(int width, int height, char* classname);

GtkWidget* gtk_create_button(char* label, int width, int height, char* classname);
GtkWidget* gtk_create_input(char* placeholder, int width, int height, char* classname);

GtkWidget* gtk_create_eventbox(char* classname);

GtkWidget* gtk_create_label(char* label_text, int width, int height, char* classname);

GtkWidget* gtk_create_scrolled_window(int width, int height, char* classname);
void gtk_align_center(GtkWidget* parent, GtkWidget* child);


GtkWidget* gtk_box_add_item(GtkWidget* box, char* text);

void safe_label_set_text(GtkLabel *label, const gchar *text);

gboolean _schedule_draw_room_list(gpointer user_data);
gboolean _schedule_draw_chat_list(gpointer user_data);

GtkWidget* get_emoji_img(char* emoji_name, int size);
GtkWidget* get_react_icon_img(chat_react_type type, int size);

char *format_kakao_style(char *datetime_str);

#endif

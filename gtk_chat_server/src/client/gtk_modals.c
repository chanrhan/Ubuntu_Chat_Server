#include "gtk_draw.h"
#include "gtk_callback.h"
#include "gtk_modal.h"

static GtkWidget *_react_popover = NULL;
static GtkWidget *_emoji_popover = NULL;

gtk_modal_add_room md_add_room;
gtk_modal_search_user md_search_user;
gtk_modal_snackbar md_snackbar;
gtk_modal_chat_context_menu md_chat_context_menu;
gtk_modal_react_context_menu md_react_context_menu;
gtk_modal_delete_context_menu md_delete_context_menu;

GtkWidget* gtk_create_modal_window(GtkWidget* parent, char* title, int width, int height){
    GtkWidget* modal = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(modal), title);
    gtk_window_set_position(GTK_WINDOW(modal), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(modal), width, height);

    g_signal_connect(modal, "delete-event", G_CALLBACK(on_modal_delete_event), NULL); // 종료 콜백 

    gtk_window_set_transient_for(GTK_WINDOW(modal), GTK_WINDOW(parent));
    gtk_window_set_modal(GTK_WINDOW(modal), TRUE);
    return modal;
}

void init_md_snackbar(GtkWidget* parent){
    int MAX_WIDTH = 300;
    int MAX_HEIGHT = 200;
    
    md_snackbar.window = gtk_dialog_new();
    // g_signal_connect(md_add_room.window, "delete-event", G_CALLBACK(g_callback_close_add_room), NULL); // 종료 콜백 

    // md_snackbar.text = gtk_create_label("", 200, 150, "snackbar");
    // gtk_container_add(md_snackbar.window, md_snackbar.text);
}

void show_md_react_context_menu(GtkWidget *anchor, int chat_id)
{
    md_react_context_menu.curr_chat_id = chat_id;
    printf("show react popover: %p\n", _react_popover);
    // 1) 팝오버가 아직 생성되지 않았다면, anchor 기준으로 한 번만 생성
    if (!_react_popover) {
        _react_popover = gtk_popover_new(anchor);
        gtk_popover_set_modal   (GTK_POPOVER(_react_popover), TRUE);
        // gtk_popover_set_has_grab(GTK_POPOVER(_react_popover), TRUE);

        // 내부 컨테이너(HBox) 만들기
        GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(_react_popover), hbox);

        // 버튼(아이콘) 생성 & pack
        chat_react_type types[] = {
            LOVE, LIKE, CHECK, LAUGH, SURPRISE, SAD
        };

        for (int i = 0; i < G_N_ELEMENTS(types); i++) {
            GtkWidget *btn = gtk_button_new();
            gtk_button_set_image(
                GTK_BUTTON(btn),
                get_react_icon_img(types[i], 24)
            );
            gtk_button_set_relief(GTK_BUTTON(btn), GTK_RELIEF_NONE);

            // 클릭 시그널 연결 예시
            g_signal_connect(btn, "clicked",
                             G_CALLBACK(g_callback_select_react_icon),
                             GINT_TO_POINTER(types[i]));
            g_signal_connect_swapped(btn, "clicked",
                    G_CALLBACK(gtk_widget_hide),
                    _react_popover);

            gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 0);
        }

        // 초기 렌더링
        gtk_widget_show_all(_react_popover);
    }
    // 2) 이미 생성된 팝오버라면, 새로운 anchor 에 재연결
    else {
        gtk_popover_set_relative_to(
            GTK_POPOVER(_react_popover),
            anchor
        );
    }

    // 3) 팝오버 열기
    gtk_widget_show(_react_popover);
}

void show_md_emoji_context_menu()
{
    GtkWidget* anchor = service_page->btn_open_emoji_context;

    if (!_emoji_popover) {
        _emoji_popover = gtk_popover_new(anchor);
        gtk_popover_set_modal   (GTK_POPOVER(_emoji_popover), TRUE);
        // gtk_popover_set_has_grab(GTK_POPOVER(_react_popover), TRUE);

        GtkWidget *grid = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(_emoji_popover), grid);

        const char *folder_path = "src/resources/images/emoji/";  // 현재 디렉토리
        struct dirent *entry;
        DIR *dp = opendir(folder_path);

        if (dp == NULL) {
            perror("opendir");
            return;
        }
        int r=0, c=0;
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] != '.') {
                // printf("emoji name:%s\n", entry->d_name);
                GtkWidget *btn_emoji = gtk_button_new();
                GtkWidget* img = get_emoji_img(entry->d_name, 64);
                if(!img){
                    continue;
                }
                gtk_button_set_image(
                    GTK_BUTTON(btn_emoji),
                    img
                );
                gtk_button_set_relief(GTK_BUTTON(btn_emoji), GTK_RELIEF_NONE);

                g_signal_connect(btn_emoji, "clicked",
                                G_CALLBACK(g_callback_select_emoji),
                                (void*)entry->d_name);
                g_signal_connect_swapped(btn_emoji, "clicked",
                        G_CALLBACK(gtk_widget_hide),
                        _emoji_popover);

                // gtk_box_pack_start(GTK_BOX(grid), btn_emoji, FALSE, FALSE, 0);
                gtk_grid_attach(GTK_GRID(grid), btn_emoji, c, r, 1, 1);
                ++c;
                if(c % 4 == 0){
                    c=0;
                    ++r;
                }
            }
        }

        // 초기 렌더링
        gtk_widget_show_all(_emoji_popover);
    }
    else {
        gtk_popover_set_relative_to(
            GTK_POPOVER(_emoji_popover),
            anchor
        );
    }

    // 3) 팝오버 열기
    gtk_widget_show(_emoji_popover);
}

void init_md_chat_context_menu(GtkWidget* parent){
    int MAX_WIDTH = 300;
    int MAX_HEIGHT = 200;
    
    GtkWidget* menu, *item_reply, *item_react, *item_delete;
    menu = gtk_menu_new();

    g_signal_connect(menu, "selection-done", G_CALLBACK(on_deactivate_chat_context_menu), NULL);

    // g_signal_connect(menu, "hide", G_CALLBACK(on_hide_chat_context_menu), NULL);
    md_chat_context_menu.window = menu;

    item_reply = gtk_menu_item_new_with_label("reply");
    item_react = gtk_menu_item_new_with_label("react");
    item_delete = gtk_menu_item_new_with_label("delete");

    g_signal_connect(item_reply, "activate", G_CALLBACK(g_callback_chat_reply), NULL);
    g_signal_connect(item_react, "activate", G_CALLBACK(g_callback_chat_react), NULL);
    g_signal_connect(item_delete, "activate", G_CALLBACK(g_callback_onclick_delete_item), NULL);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_reply);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_react);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_delete);

    gtk_widget_show_all(menu);

    md_chat_context_menu.item_reply = item_reply;
    md_chat_context_menu.item_react = item_react;
    md_chat_context_menu.item_delete = item_delete;
}


void init_md_delete_context_menu(GtkWidget* parent){
    int MAX_WIDTH = 300;
    int MAX_HEIGHT = 200;
    
    GtkWidget* menu, *item_self;
    menu = gtk_menu_new();

    g_signal_connect(menu, "selection-done", G_CALLBACK(on_deactivate_delete_context_menu), NULL);

    // g_signal_connect(menu, "hide", G_CALLBACK(on_hide_chat_context_menu), NULL);
    md_delete_context_menu.window = menu;

    item_self = gtk_menu_item_new_with_label("이 기기에서 삭제");
    md_delete_context_menu.item_delete_all = gtk_menu_item_new_with_label("모든 대화 상대에게서 삭제");

    g_signal_connect(md_delete_context_menu.item_delete_all, "activate", G_CALLBACK(g_callback_delete_chat_all), NULL);
    g_signal_connect(item_self, "activate", G_CALLBACK(g_callback_delete_chat_self), NULL);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), md_delete_context_menu.item_delete_all);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_self);

    gtk_widget_show_all(menu);
}

void init_md_add_room(GtkWidget* parent){
    int MAX_WIDTH = 600;
    int MAX_HEIGHT = 400;

    md_add_room.window = gtk_create_modal_window(parent,  "Create Room", MAX_WIDTH, MAX_HEIGHT);
    g_signal_connect(md_add_room.window, "delete-event", G_CALLBACK(g_callback_close_add_room), NULL); // 종료 콜백 


    GtkWidget* cont = gtk_create_vbox(MAX_WIDTH, MAX_HEIGHT, "md_add_room");
    gtk_container_add(md_add_room.window, cont);

    GtkWidget* room_name_box, *invite_box, *bottom_box, *room_name_label, *invite_label, *inivited_list;
    // room_name_box
    room_name_box = gtk_create_vbox(600, 100, "room_name_box");
    gtk_container_add(cont, room_name_box);

    room_name_label = gtk_create_label("방 이름",500, 40, "room_name_label");
    gtk_container_add(room_name_box, room_name_label);
    md_add_room.inp_room_name = gtk_create_input("input room name",500, 60, "inp_room_name");
    gtk_container_add(room_name_box, md_add_room.inp_room_name);

    // invite_box
    invite_box = gtk_create_vbox(600, 200, "invite_box");
    gtk_container_add(cont, invite_box);

    invite_label = gtk_create_label("초대할 사람",600, 50, "invite_label");
    gtk_container_add(invite_box, invite_label);
    md_add_room.inp_search_user = gtk_create_input("search users",600, 50, "inp_search_user");
    g_signal_connect(md_add_room.inp_search_user, "changed", G_CALLBACK(g_callback_search_user), NULL); // 입력 변경 시 콜백
    gtk_container_add(invite_box, md_add_room.inp_search_user);

    md_add_room.user_liststore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    // GtkTreeIter iter;

    // const char *items[] = {"apple", "apricot", "banana", "blueberry", "cherry", "grape", "melon", "orange"};
    // for (int i = 0; i < sizeof(items)/sizeof(items[0]); i++) {
    //     gtk_list_store_append(md_add_room.user_liststore, &iter);
    //     gtk_list_store_set(liststore, &iter, 0, items[i], -1);
    // }

    GtkEntryCompletion *completion = gtk_entry_completion_new();
    gtk_entry_completion_set_model(completion, NULL);
    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(md_add_room.user_liststore));
    gtk_entry_completion_set_text_column(completion, 0); // 0번째 컬럼을 표시용으로 지정
    gtk_entry_completion_set_inline_completion(completion, TRUE); // 입력 중 자동완성 제안

    gtk_entry_set_completion(GTK_ENTRY(md_add_room.inp_search_user), completion);

    g_signal_connect(completion, "match-selected", G_CALLBACK(on_select_searched_user), NULL);

    // invite_list
    md_add_room.user_list_box = gtk_create_hbox(600, 100, "user_list_box");
    gtk_container_add(invite_box, md_add_room.user_list_box);

    // bottom_box
    bottom_box = gtk_create_hbox(600, 100, "bottom_box");
    gtk_container_add(cont, bottom_box);

    md_add_room.btn_cancel = gtk_create_button("취소",300, 100, "btn_cancel");
    gtk_container_add(bottom_box, md_add_room.btn_cancel);
    md_add_room.btn_submit = gtk_create_button("생성",300, 100, "btn_submit");
    g_signal_connect(md_add_room.btn_submit, "clicked", G_CALLBACK(g_callback_add_room), NULL);

    gtk_container_add(bottom_box, md_add_room.btn_submit);
    
}


void init_modal(GtkWidget* parent){
    init_md_add_room(parent);
    // init_md_snackbar(parent);
    init_md_chat_context_menu(parent);
    init_md_delete_context_menu(parent);
}

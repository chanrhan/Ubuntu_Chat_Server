// #include "client.h"
#include "client_sock.h"
#include "gtk_callback.h"
// #include "client_sock.h"
#include "gtk_draw.h"
#include "client_recv_cmd.h"
#include "gtk_modal.h"
#include "gtk_utils.h"

#pragma region onclick_chat
int get_clicked_chat_id(GtkWidget* widget){
    int *id = (int *)g_object_get_data(G_OBJECT(widget), "chat_id");
    return *id;
}

int get_clicked_chat_item_index(GtkWidget* widget){
    int *index = (int *)g_object_get_data(G_OBJECT(widget), "index");
    return *index;
}

int g_callback_onclick_chat_item(GtkWidget* widget, GdkEventButton *event, gpointer data){
    printf("click\n");
    int chat_id = get_clicked_chat_id(widget);
    // printf("chat_id : %d, butotn: %d\n", chat_id, event->button);
    if(event->type == GDK_BUTTON_PRESS && event->button == 3){
        // 우클릭 (right click)
        md_chat_context_menu.curr_chat_id = chat_id;
        gtk_menu_popup_at_pointer(md_chat_context_menu.window, (GdkEvent *)event);
    }else{

    }
}
int g_callback_onclick_delete_item(GtkWidget* widget, GdkEventButton *event, gpointer data){
    int chat_id = md_chat_context_menu.curr_chat_id;
    md_delete_context_menu.curr_chat_id = chat_id;
    int is_mine = 0;
    for(int i=0;i<service_page->chat_list_size;++i){
        chat_vo item = service_page->chat_list_data[i];
        if(item.chat_id == chat_id){
            // printf("strcmp:%s|%s\n", item.owner_id, cli_session.id);
            if(strcmp(item.owner_id, cli_session.id) != 0){
                gtk_widget_hide(md_delete_context_menu.item_delete_all);
            }else{

                gtk_widget_show(md_delete_context_menu.item_delete_all);
            }
            gtk_widget_show_all(md_delete_context_menu.window);
            break;
        }
    }
    gtk_menu_popup_at_pointer(md_delete_context_menu.window, (GdkEvent *)event);
    return SUCCESS;
}

int g_callback_chat_reply(GtkWidget* widget, gpointer data){
    int reply_id = md_chat_context_menu.curr_chat_id;
    // printf("reply : %d\n", reply_id);
    service_page->curr_reply_id = reply_id;

    for(int i=0;i<service_page->chat_list_size;++i){
        chat_vo chat = service_page->chat_list_data[i];
        if(chat.chat_id == reply_id){
            char reply_user_text[32];
            snprintf(reply_user_text, 32, "%s에게 답장", chat.owner_name);
            gtk_label_set_text(service_page->reply_user_id_label, reply_user_text);
            gtk_label_set_text(service_page->reply_text_label, chat.text);
            gtk_style_class_toggle(service_page->reply_box, "reply_mode", TRUE);
            break;
        }
    }
    return SUCCESS;
}

int g_callback_select_react_icon(GtkWidget* widget, gpointer data){
    int react_type = (int)data;
    printf("react : %d\n", react_type);

    req_packet_t req = {
        .header.type = SEND_CHAT_REACT
    };

    snprintf(req.argv[0], sizeof(req.argv[0]), "%d", service_page->curr_room_id);
    snprintf(req.argv[1], sizeof(req.argv[1]), "%d", md_react_context_menu.curr_chat_id);
    snprintf(req.argv[2], sizeof(req.argv[2]), "%d", react_type);

    if(send(sock_fd, &req, sizeof(req_packet_t), 0) < 0){
        perror("send req");
        return FAIL;
    }

}
int g_callback_chat_react(GtkWidget* widget, GdkEventButton *event, gpointer data){
    int chat_id = md_chat_context_menu.curr_chat_id;

    for(int i=0;i<service_page->chat_list_size;++i){
        chat_vo chat = service_page->chat_list_data[i];
        if(chat.chat_id == chat_id){
            GtkWidget* chat_item = get_chat_item(i);

            GList *children, *iter;

            children = gtk_container_get_children(chat_item);
            if(!children){
                break;
            }
            int k=0;
            for (iter = children; iter != NULL; iter = g_list_next(iter)) {
                if(k == 0){
                    show_md_react_context_menu(iter->data, chat_id);
                    break;
                }
                ++k;
            }
            break;
        }
    }
}
int g_callback_chat_delete(GtkWidget* widget, gpointer data){

}

gboolean on_deactivate_chat_context_menu(GtkWidget *widget, GdkEventCrossing *event, gpointer data){
    // printf("deactivate context menu\n");
    md_chat_context_menu.curr_chat_id = -1;
}

gboolean on_deactivate_delete_context_menu(GtkWidget *widget, GdkEventCrossing *event, gpointer data){
    // printf("deactivate context menu\n");
    md_chat_context_menu.curr_chat_id = -1;
}


gboolean on_hide_chat_context_menu(GtkWidget *widget, GdkEventCrossing *event, gpointer data){
    // printf("hide context menu\n");

    md_chat_context_menu.curr_chat_id = -1;
}

// hover in
gboolean on_enter_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer data) {
    int chat_id = get_clicked_chat_id(widget);
    // printf("hover enter: %d\n", chat_id);
    // int index = get_clicked_chat_item_index(widget);
    // if(index < service_page->chat_list_size){
    //     GtkWidget* item = service_page->chat_item_list[index];
    //     if(item){
    //         gtk_style_class_toggle(item, "react", TRUE);
    //     }
    // }
    gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT, TRUE);
    return FALSE;
}

// hover out
gboolean on_leave_notify(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data) {
    int chat_id = get_clicked_chat_id(widget);
    // printf("hover leave: %d\n", chat_id);

    // int index = get_clicked_chat_item_index(widget);
    // if(index < service_page->chat_list_size){
    //     GtkWidget* item = service_page->chat_item_list[index];
    //     if(item){
    //         gtk_style_class_toggle(item, "react", FALSE);
    //     }
    // }

    gtk_widget_unset_state_flags(widget, GTK_STATE_FLAG_PRELIGHT);
    return FALSE;
}

int g_callback_delete_chat_all(GtkWidget* widget, gpointer data){
    int chat_id = md_delete_context_menu.curr_chat_id;
    // printf("delet all\n");

    req_packet_t req = {
        .header.type = DELETE_CHAT
    };

    snprintf(req.argv[0], sizeof(req.argv[0]), "%d", service_page->curr_room_id);
    snprintf(req.argv[1], sizeof(req.argv[1]), "%d", chat_id);
    snprintf(req.argv[2], sizeof(req.argv[3]), "%d", 1);

    if(send(sock_fd, &req, sizeof(req_packet_t), 0) < 0){
        perror("send req");
        return FAIL;
    }
}

int g_callback_delete_chat_self(GtkWidget* widget, gpointer data){
    int chat_id = md_delete_context_menu.curr_chat_id;
    // printf("delete self\n");

    req_packet_t req = {
        .header.type = DELETE_CHAT
    };

    snprintf(req.argv[0], sizeof(req.argv[0]), "%d", service_page->curr_room_id);
    snprintf(req.argv[1], sizeof(req.argv[1]), "%d", chat_id);
    snprintf(req.argv[2], sizeof(req.argv[3]), "%d", 0);
    
    if(send(sock_fd, &req, sizeof(req_packet_t), 0) < 0){
        perror("send req");
        return FAIL;
    }
}

#pragma endregion

#pragma region add_room
int g_callback_search_user(GtkWidget* widget, gpointer data){
    char* search_text = gtk_entry_get_text(md_add_room.inp_search_user);
    // printf("search user: %s\n", search_text);
    if(is_empty_string(search_text) == SUCCESS){
        gtk_list_store_clear(md_add_room.user_liststore);
        return FAIL;
    }
    req_packet_t req = {
        .header.type = Search_User_When_Create_Room
    };
    snprintf(req.header.access_token, sizeof(req.header.access_token), "%s", cli_session.access_token);
    snprintf(req.argv[0], sizeof(req.argv[0]), "%s", search_text);
    // snprintf(req.argv[0], sizeof(req.argv[0]), "%s", cli_session.id);

    res_packet_t res;

    printf("send\n");
    send_and_receive(Search_User_When_Create_Room, &req, &res);
    printf("receive\n");

    user_vo* users = (user_vo*)res.data;
    // 자동완성 목록들 업데이트
    GtkTreeIter iter;

    gtk_list_store_clear(md_add_room.user_liststore);

    for (int i = 0; i < res.data_len; i++) {
        // printf("%s | %s\n", users[i].id, users[i].name);
        gtk_list_store_append(md_add_room.user_liststore, &iter);
        gtk_list_store_set(md_add_room.user_liststore, &iter, 
                                0, users[i].name ? users[i].name : "", 
                                1, users[i].id ? users[i].id : "",
                                -1);
    }
}

int on_select_searched_user(GtkEntryCompletion *completion, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data){
    char *id, *name;
    gtk_tree_model_get(model, iter, 0, &name, 1, &id, -1);

    user_t* new_user = malloc(sizeof(user_t));
    new_user->id = strdup(id);
    new_user->name = strdup(name);
    new_user->next = NULL;

    if(md_add_room.invite_user_list == NULL){
        md_add_room.invite_user_list = new_user;
    }else{
        user_t* curr = md_add_room.invite_user_list;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = new_user;
    }

    GtkWidget* item_box = gtk_create_hbox(75, 50, "user_item");

    GtkWidget *label = gtk_create_label(name, 50, 50, "text");
    gtk_container_add(item_box, label);
    GtkWidget *button = gtk_create_button("X", 25, 25, "btn");
    gtk_container_add(item_box, button);

    
    gtk_box_pack_start(GTK_BOX(md_add_room.user_list_box), item_box, FALSE, FALSE, 5);
    gtk_widget_show_all(item_box);

    g_free(id);
    g_free(name);

    // gtk_entry_set_text(md_add_room.inp_search_user, "");

    return FALSE;
}

void print_invite_user_list(){
    user_t* curr = md_add_room.invite_user_list;
    while(curr != NULL){
        printf("invite user: %s\n", curr->id);
        curr = curr->next;
    }
}

int g_callback_remove_list_user_item(GtkWidget* widget, gpointer data){

}


int g_callback_close_add_room(GtkWidget* widget, gpointer data){
    gtk_list_store_clear(md_add_room.user_liststore);
}

int g_callback_open_add_room(GtkWidget* widget, gpointer data){
    // md_add_room.invite_user_list = NULL;
    gtk_widget_show_all(md_add_room.window);
}

int g_callback_add_room(GtkWidget* widget, gpointer data){
    // print_invite_user_list();
    // return 1;
    char* room_name = gtk_entry_get_text(md_add_room.inp_room_name);
    if(room_name == NULL || strcmp(room_name, "") == 0){
        return FALSE;
    }

    req_packet_t req = {
        .header.type = CREATE_ROOM
    };
    snprintf(req.argv[0], sizeof(req.argv[0]), "%s", room_name);
    snprintf(req.argv[2], sizeof(req.argv[2]), "%s", cli_session.id);
    // printf("cli user id: %s\n", cli_session.id);
    int i=3;
    user_t* curr = md_add_room.invite_user_list;
    while(curr != NULL && i < 16){
        snprintf(req.argv[i], sizeof(req.argv[i]), "%s", curr->id);
        // printf("invite id: %s\n", req.argv[i]);
        curr = curr->next;
        ++i;
    }
    snprintf(req.argv[1], sizeof(req.argv[1]), "%d", i - 2);


    res_packet_t res;

    if(send_and_receive(CREATE_ROOM, &req, &res) == FAIL){
        gtk_widget_hide(md_add_room.window);
        return FAIL;
    }
    gtk_widget_hide(md_add_room.window);
    
    reload_service_page();
}
#pragma endregion


int g_callback_select_room(GtkWidget* widget, gpointer data){
    int* room_id = (int *)g_object_get_data(G_OBJECT(widget), "room_id");
    int b = *room_id;
    reload_chat_list(b);
}

int g_callback_open_room_menu(GtkWidget* widget, gpointer data){

}

int g_callback_search_room(GtkWidget* widget, gpointer data){

}

int g_callback_search_chat(GtkWidget* widget, gpointer data){

}

int g_callback_show_emoji_context_menu(GtkWidget* widget, gpointer data){
    show_md_emoji_context_menu();
}

int g_callback_select_emoji(GtkWidget* widget, gpointer data){
    char* emoji_name = (char*)data;
    printf("emoji name : %s\n", emoji_name);

    if(is_empty_string(emoji_name) == SUCCESS){
        return FAIL;
    }

    req_packet_t req = {
        .header.type = SEND_CHAT
    };

    snprintf(req.argv[0], sizeof(req.argv[0]), "%d", EMOJI);
    snprintf(req.argv[1], sizeof(req.argv[1]), "%d", service_page->curr_room_id);
    snprintf(req.argv[2], sizeof(req.argv[2]), "%s", emoji_name);
    snprintf(req.argv[3], sizeof(req.argv[3]), "%d", service_page->curr_reply_id);
    // printf("send (reply debug) : %s, %s, %s\n", req.argv[0], req.argv[1], req.argv[2]);

    // printf("send chat\n");
    if(send(sock_fd, &req, sizeof(req_packet_t), 0) < 0){
        perror("send req");
        return FAIL;
    }
}


int g_callback_send_chat(GtkWidget* widget, gpointer data){
    char* text = gtk_entry_get_text(service_page->inp_chat);
    if(is_empty_string(text) == SUCCESS){
        return FAIL;
    }

    req_packet_t req = {
        .header.type = SEND_CHAT
    };

    snprintf(req.argv[0], sizeof(req.argv[0]), "%d", CHAT);
    snprintf(req.argv[1], sizeof(req.argv[1]), "%d", service_page->curr_room_id);
    snprintf(req.argv[2], sizeof(req.argv[2]), "%s", text);
    snprintf(req.argv[3], sizeof(req.argv[3]), "%d", service_page->curr_reply_id);
    // printf("send (reply debug) : %s, %s, %s\n", req.argv[0], req.argv[1], req.argv[2]);

    // printf("send chat\n");
    if(send(sock_fd, &req, sizeof(req_packet_t), 0) < 0){
        perror("send req");
        return FAIL;
    }
    gtk_entry_set_text(service_page->inp_chat, "");
    // printf("receive chat \n");

    // reload_chat_list(cli_session.curr_room_id);
}

int g_callback_login(GtkWidget* widget, gpointer data){
    req_packet_t req;
    req.header.type = LOGIN;

    char* id = gtk_entry_get_text(login_page->id_input);
    char* pwd = gtk_entry_get_text(login_page->pwd_input);

    snprintf(req.argv[0], sizeof(req.argv[0]), "%s", id);
    snprintf(req.argv[1], sizeof(req.argv[1]), "%s", pwd);

    res_packet_t res;
    if(send_and_receive(LOGIN, &req, &res) == FAIL){
        return FAIL;
    }
    
    return recv_cmd_login(&res);
}

int g_callback_signup(GtkWidget* widget, gpointer data){
    req_packet_t req;
    req.header.type = SIGNUP;

    char* id = gtk_entry_get_text(signup_page->id_input);
    char* pwd = gtk_entry_get_text(signup_page->pwd_input);
    char* name = gtk_entry_get_text(signup_page->name_input);
    
    snprintf(req.argv[0], sizeof(req.argv[0]), "%s", id);
    snprintf(req.argv[1], sizeof(req.argv[1]), "%s", pwd);
    snprintf(req.argv[2], sizeof(req.argv[2]), "%s", name);

    res_packet_t res;
    if(send_and_receive(SIGNUP, &req, &res) == FAIL){
        return FAIL;
    }
    
    return recv_cmd_signup(&res);
}

int g_callback_move_login_page(GtkWidget* widget, gpointer data){
    gtk_stack_set_visible_child_name(page_stack, "LOGIN");

}

int g_callback_move_signup_page(GtkWidget* widget, gpointer data){
    gtk_stack_set_visible_child_name(page_stack, "SIGNUP");
}

gboolean on_realize(gpointer data){

    return FALSE;
}


int gtk_exit(GtkWidget* widget, gpointer data){
    printf("QUIT\n");
    close(sock_fd);

    gtk_main_quit();

    return 0;
}

void on_popup_close(GtkWidget *widget, gpointer data) {
    GtkWidget *popup = GTK_WIDGET(data);
    gtk_widget_hide(popup);  // gtk_widget_destroy 시 위젯이 완전히 삭제되기 때문에 hide 사용 
}

gboolean on_modal_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data){
    printf("on delet-event\n");
    gtk_widget_hide(widget);
    return TRUE;
}

gboolean on_key_press(GtkWidget *widget,GdkEventKey *event, gpointer user_data){
    if (event->keyval == GDK_KEY_Escape) {
        // g_print("Escape 키 눌림!\n");
        // 예: 모달 닫기
        // gtk_widget_destroy(GTK_WIDGET(user_data));
        service_page->curr_reply_id = -1;
        gtk_label_set_text(service_page->reply_user_id_label, "");
        gtk_label_set_text(service_page->reply_text_label, "");
        gtk_style_class_toggle(service_page->reply_box, "reply_mode", FALSE);
        return TRUE;  // 이벤트를 처리했음을 GTK에 알림
    }
    return FALSE;     // 다른 키 이벤트는 기본 처리
}
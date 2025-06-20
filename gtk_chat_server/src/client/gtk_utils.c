#include "gtk_draw.h"
#include "gtk_modal.h"
#include "gtk_utils.h"
#include "gtk_callback.h"
#include "client_recv_cmd.h"

int reload_service_page(){
    int first_room_id = reload_room_list();
    printf("first room id : %d\n", first_room_id);
    
    cli_session.curr_room_id = first_room_id;
    if(first_room_id > 0){
        reload_chat_list(first_room_id);

    }
    
    return SUCCESS;
}

int reload_room_list(){
     req_packet_t req = {
        .header.type = UPDATE_ROOM_LIST
    };
    printf("---reload ROOM list---\n");

    res_packet_t res;
    if(send_and_receive(UPDATE_ROOM_LIST, &req, &res) == FAIL){
        perror("reload_room_list");
        remove_all_children(service_page->room_list);
        gtk_widget_show_all(service_page->room_list);

        remove_all_children(service_page->chat_list);
        gtk_widget_show_all(service_page->chat_list);

        return -1;
    }
    // printf("receive room_list\n");

    remove_all_children(service_page->room_list);
    if(res.data == NULL || res.data_len <= 0){
        printf("(room list) data is NULL\n");
        // free(service_page->room_list_data);
        service_page->room_list_data = NULL;
        service_page->room_list_size = 0;
        return -1;
    }
    room_vo* rooms = (room_vo*)res.data;
    service_page->room_list_size = res.data_len;
    service_page->room_list_data = malloc(sizeof(room_vo) * res.data_len);
    memcpy(service_page->room_list_data, rooms, sizeof(room_vo) * res.data_len);

    int first_room_id = rooms[0].room_id;
    // draw_room_list();
    g_idle_add(_schedule_draw_room_list, NULL);

    return first_room_id;
}

void reload_chat_list(int room_id){
    req_packet_t req = {
        .header.type = UPDATE_CHAT_LIST
    };

    snprintf(req.argv[0], sizeof(req.argv[0]), "%d", room_id);
    
    printf("---reload CHAT list (%d)---\n", room_id);
    res_packet_t res;
    if(send_and_receive(UPDATE_CHAT_LIST, &req, &res) == FAIL){
        perror("reload_chat_list");
        remove_all_children(service_page->chat_list);
        gtk_widget_show_all(service_page->chat_list);

        return;
    }
    printf("receive chat_list\n");

    service_page->curr_room_id = room_id;
    char* room_name;
    if(service_page->room_list_data == NULL){
        remove_all_children(service_page->chat_list);
        printf("room list data is NULL\n");
        return;
    }

    for(int i=0;i<service_page->room_list_size;++i){
        // printf("room_name : %s\n", service_page->room_list_data[i].room_name);
        if(service_page->room_list_data[i].room_id == room_id){
            room_name = service_page->room_list_data[i].room_name;
            break;
        }
    }

    gtk_label_set_text(service_page->room_name_box, room_name);

    if(service_page == NULL || service_page->chat_list == NULL){
        remove_all_children(service_page->chat_list);
        printf("[Error] Chat List is Null\n");
        return;
    }
    // printf("before remove_all_children\n");
    // remove_all_children(service_page->chat_list);
    // printf("after remove_all_children\n");
    if(res.data == NULL){
        remove_all_children(service_page->chat_list);
        printf("[Error] result Data is Null\n");
        return;
    }

    chat_vo* chats = (chat_vo*)res.data;
    service_page->chat_list_size = res.data_len;
    printf("chat list size (init) : %d\n", service_page->chat_list_size);
    service_page->chat_list_data = malloc(sizeof(chat_vo) * res.data_len);
    memcpy(service_page->chat_list_data, chats, sizeof(chat_vo) * res.data_len);

    // draw_chat_list();
     g_idle_add(_schedule_draw_chat_list, NULL);
}


GtkWidget* gtk_create_room_item(room_vo* vo){
    GtkWidget* item = gtk_event_box_new();
    gtk_style_class_toggle(item, "room_item", TRUE);
    // gtk_widget_set_size_request(item, 250, 50);
    // printf("room_id : %d\n", vo->room_id);

    int* room_id = g_new(int, 1);
    *room_id = vo->room_id;
    g_object_set_data(G_OBJECT(item), "room_id", room_id);

    // GtkWidget* item = gtk_create_vbox(250, 50, "room_item");
    g_signal_connect(item, "button-press-event", G_CALLBACK(g_callback_select_room), NULL);
    
    GtkWidget* hbox = gtk_create_hbox(200, 60, "");
    gtk_container_add(item, hbox);

    // vbox1 : 방 이름, 마지막 채팅
    GtkWidget* vbox1 = gtk_create_vbox(150, 60, "");
    gtk_container_add(hbox, vbox1);

    // hbox2 : 방 이름 , 멤버 수 
    GtkWidget* hbox2 = gtk_create_hbox(150, 20, "");
    gtk_container_add(vbox1, hbox2);

    GtkWidget* room_name = gtk_create_label(vo->room_name,100, 20, "room_name");
    char mn[16];
    snprintf(mn, 16, "%d", vo->member_num);
    GtkWidget* member_num = gtk_create_label(mn,20, 20, "member_num");
    gtk_box_pack_start(hbox2, room_name, FALSE, FALSE, 0);
    gtk_box_pack_start(hbox2, member_num, FALSE, FALSE, 0);


    GtkWidget* hbox3 = gtk_create_hbox(150, 40, "");
    gtk_container_add(vbox1, hbox3);

    GtkWidget* last_chat_text = gtk_create_label(vo->last_chat, 100, 40, "last_chat_text");
    gtk_box_pack_start(hbox3, last_chat_text, FALSE, FALSE, 0);


    // vbox2 : 마지막 채팅 시간, 안읽은 채팅 수 
    GtkWidget* vbox2 = gtk_create_vbox(50, 60, "");
    gtk_box_pack_start(hbox, vbox2, FALSE, FALSE, 0);

    char* ktm = format_kakao_style(vo->last_chat_time);
    GtkWidget* last_chat_time = gtk_create_label(ktm,50, 30, "last_chat_time");
    gtk_container_add(vbox2, last_chat_time);

    char* uct = malloc(16);
    if(vo->updated_chat_count > 0){
        snprintf(uct, 16, "%d", vo->updated_chat_count);
        GtkWidget* hbox4 = gtk_create_hbox(50, 30, "");
        gtk_container_add(vbox2, hbox4);

        GtkWidget* updated_chat_count = gtk_create_label(uct,30, 30, "updated_chat_count");
        gtk_box_pack_start(hbox4, updated_chat_count, FALSE, FALSE, 0);
    }
    free(ktm);
    ktm = NULL;

    return item;
}

GtkWidget* create_react_set(chat_react_type type, int count){
    GtkWidget* hbox = gtk_create_hbox(25, 25, "");
    gtk_box_pack_start(hbox, get_react_icon_img(type, 20), FALSE, FALSE, 0);

    char cnt[8];
    snprintf(cnt, 8 , "%d", count);
    GtkWidget* label = gtk_create_label(cnt, 25, 25, "");
    gtk_box_pack_start(hbox, label, FALSE, FALSE, 0);
    return hbox;
}

// // chat list (w 300 h 400)
GtkWidget* gtk_create_chat_item(int index, chat_vo* vo, int is_mine){
    // char chat_item_classname[16];
    // snprintf(chat_item_classname, 16, "chat_item %s", is_mine == 0 ? "self" : "");
    // printf("chat item classname: %s\n", chat_item_classname);

    GtkWidget* event_box = gtk_event_box_new();
    gtk_style_class_toggle(event_box, "chat_event_box", TRUE);

    int* chat_id = g_new(int, 1);
    *chat_id = vo->chat_id;
    g_object_set_data(G_OBJECT(event_box), "chat_id", chat_id); 

    int* idx = g_new(int, 1);
    *idx = index;
    g_object_set_data(G_OBJECT(event_box), "index", idx); 

    g_signal_connect(event_box, "button-press-event", G_CALLBACK(g_callback_onclick_chat_item), NULL);
    g_signal_connect(event_box, "enter-notify-event", G_CALLBACK(on_enter_notify), NULL);
    g_signal_connect(event_box, "leave-notify-event", G_CALLBACK(on_leave_notify), NULL);

    int item_height = 45;

    GtkWidget* item = gtk_create_vbox(200, item_height, "");
    gtk_container_add(event_box, item);

    GtkWidget* name_hbox =  gtk_create_hbox(100, 5, "");
    gtk_container_add(item, name_hbox);
    
    GtkWidget* owner_name = NULL;
    if(is_mine == 0){
        owner_name = gtk_create_label("", 100, 5, "owner_name");
        
        gtk_box_pack_end(name_hbox, owner_name, FALSE, FALSE, 0);
    }else{
        owner_name = gtk_create_label(vo->owner_name, 100, 5, "owner_name");

        gtk_box_pack_start(name_hbox, owner_name, FALSE, FALSE, 0);
    }

    if(vo->reply_chat_id >= 0){
        for(int i=0;i < service_page->chat_list_size;++i){
            if(service_page->chat_list_data[i].chat_id == vo->reply_chat_id){
                GtkWidget* reply_hbox = gtk_create_vbox(200, 35, "chat_reply_box");
                gtk_container_add(item, reply_hbox);
                char reply_user_txt[32];
                snprintf(reply_user_txt, 32, "%s에게 답장", service_page->chat_list_data[i].owner_name);

                char reply_txt[64];
                snprintf(reply_txt, 64, "%s", service_page->chat_list_data[i].text);

                GtkWidget* reply_user_id = gtk_create_label(reply_user_txt, 200, 10, "");
                GtkWidget* reply_text = gtk_create_label(reply_txt, 200, 25, "");
                gtk_container_add(reply_hbox, reply_user_id);
                gtk_container_add(reply_hbox, reply_text);
                item_height += 25;
                break;
            }
        }        
    }
    
    GtkWidget* hbox = gtk_create_hbox(200, 25, "");
    char* nrc = malloc(16);
    if(vo->non_read_count > 0){
        snprintf(nrc, 16, "%d", vo->non_read_count);
    }else{
        snprintf(nrc, 16, "");
    }
    GtkWidget* non_read_count = gtk_create_label(nrc, 10, 25, "non_read_count");

    GtkWidget* content = NULL;
    if(vo->deleted == 1){
        content = gtk_create_label("(!) 삭제된 메세지입니다", 150, 25, is_mine == 0 ? "chat_item_self" : "chat_item");
    }else{
        switch(vo->chat_type){
            case CHAT:
                content = gtk_create_label(vo->text, 100, 25, is_mine == 0 ? "chat_item_self" : "chat_item");
            break;
            case EMOJI:
                content = get_emoji_img(vo->text, 100);
                // content = gtk_create_hbox(100, 100, "");
                // gtk_box_pack_start(content, get_emoji_img(vo->text, 100), FALSE, FALSE, 0);
                item_height += (100 - 25);
            break;
        }
    }
    
    if(is_mine == 0){
        gtk_widget_set_margin_right(non_read_count, 5);
        gtk_box_pack_end(hbox, content, FALSE, FALSE, 0);
        gtk_box_pack_end(hbox, non_read_count, FALSE, FALSE, 0);
        // gtk_box_pack_end(hbox, chat_text, FALSE, FALSE, 0);  
    }else{
        gtk_widget_set_margin_left(non_read_count, 5);
        gtk_box_pack_start(hbox, content, FALSE, FALSE, 0);
        gtk_box_pack_start(hbox, non_read_count, FALSE, FALSE, 0);
        // gtk_box_pack_start(hbox, chat_text, FALSE, FALSE, 0);  
    }
    gtk_container_add(item, hbox);

    GtkWidget* hbox1 = gtk_create_hbox(100, 10, "");
    gtk_container_add(item, hbox1);
    
    // hbox (w 100 h 10)
    char* ktm = format_kakao_style(vo->chat_time);
    GtkWidget* time_text = gtk_create_label(ktm, 50, 5, "time_text");

    if(is_mine == 0){
        gtk_box_pack_end(hbox1, time_text, FALSE, FALSE, 0);
    }else{
        gtk_box_pack_start(hbox1, time_text, FALSE, FALSE, 0);
    }

    

    if(vo->emo_total_cnt > 0){
        int react_box_width = 0;
        for(int i=0;i<6;++i){
            int count = vo->emo_list[i];
            if(count > 0){
                react_box_width += 25;
            }
        }
        GtkWidget* react_box = gtk_create_hbox(100, 25, "");
        gtk_container_add(item, react_box);
        GtkWidget* inner_react_box = gtk_create_hbox(react_box_width, 25, "react_box");
        
        for(int i=0;i<6;++i){
            int count = vo->emo_list[i];
            if(count > 0){
                gtk_box_pack_start(inner_react_box, create_react_set(react_types[i], count), FALSE, FALSE, 2);
            }
        }
        if(is_mine == 0){
            gtk_box_pack_end(react_box, inner_react_box, FALSE, FALSE, 0);
        }else{
            gtk_box_pack_start(react_box, inner_react_box, FALSE, FALSE, 0);
        }

        item_height += 25;
    }
    free(ktm);
    ktm = NULL;

    gtk_widget_set_size_request(item, 200, item_height);

    return event_box;
}

GtkWidget* get_chat_item(int index){
    GList *children, *iter;
    // printf("11\n");

    children = gtk_container_get_children(GTK_CONTAINER(service_page->chat_list));
    if(!children){
        return NULL;
    }
    int i=0;
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        if(i == index){
            return iter->data;
        }
        ++i;
    }
    return NULL;
}

void remove_all_children(GtkWidget* container_widget) {
    // printf("11\n");
    if(container_widget == NULL){
        printf("remove all children : Container is NULL\n");
        return;
    }
    if(!container_widget){
        printf("remove all children : Container is NULL 22\n");
        return;
    }
    GList *children, *iter;
    // printf("11\n");

    children = gtk_container_get_children(GTK_CONTAINER(container_widget));
    if(!children){
        return;
    }
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }

    g_list_free(children); // 일단 보류 
}

void draw_room_list(){
    printf("DRAW ROOM LIST\n");

    remove_all_children(service_page->room_list);

    int len = service_page->room_list_size;
    if(len <= 0){
        // free(service_page->room_list_data);
        service_page->room_list_data = NULL;
        service_page->room_list_size = 0;
    }else{
        for(int i=0;i<len;++i){
            // printf("%s|%d|%d|%s|%s\n", rooms[i].room_name, rooms[i].member_num, rooms[i].updated_chat_count, rooms[i].last_chat,  rooms[i].last_chat_time);
            GtkWidget* item = gtk_create_room_item(&service_page->room_list_data[i]);
            gtk_box_pack_start(service_page->room_list, item, FALSE, FALSE, 2);
        }
    }
    
    gtk_widget_show_all(service_page->room_list);
    printf("---DRAW END---\n");

}

void draw_chat_list(){
    printf("DRAW CHAT LIST\n");
    remove_all_children(service_page->chat_list);


    int len = service_page->chat_list_size;
    if(len <= 0){
        service_page->chat_list_data = NULL;
        service_page->chat_list_size = 0;
    }else{
        for(int i=0;i<len;++i){
            if(service_page->chat_list_data == NULL){
                continue;
            }
            GtkWidget* line = gtk_create_hbox(300, 50, "chat_line");
            gtk_widget_set_hexpand(line, TRUE);
            gtk_widget_set_margin_top(line, 5);

            int is_mine = strcmp(service_page->chat_list_data[i].owner_id, cli_session.id);

            GtkWidget* chat_item = gtk_create_chat_item(i, &service_page->chat_list_data[i], is_mine);

            gtk_widget_set_hexpand(chat_item, FALSE);

            // 본인 채팅이면 오른쪽, 상대 채팅이면 왼쪽으로 정렬 (아직 미구현)
            if(is_mine == 0){
                // gtk_widget_set_halign(item, GTK_ALIGN_END);
                gtk_box_pack_end(GTK_BOX(line), chat_item, FALSE, FALSE, 0);
            }else{
                // gtk_widget_set_halign(item, GTK_ALIGN_START);
                gtk_box_pack_start(GTK_BOX(line), chat_item, FALSE, FALSE, 0);
            }
            // gtk_container_add(line, item);
            // gtk_box_pack_start(GTK_BOX(line), item, TRUE, FALSE, 0);
            gtk_container_add(service_page->chat_list, line);
        }
        GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(service_page->chat_list_scroll));
        gtk_adjustment_set_value(vadj, gtk_adjustment_get_upper(vadj));
    }
    
    gtk_widget_show_all(service_page->chat_list);
    printf("---DRAW END---\n");
}

void update_room_info(int room_id, room_vo* vo){
    for(int i=0;i<service_page->room_list_size;++i){

    }
}

void show_modal_snackbar(char* text){
    if(!text){
        return;
    }
    gtk_label_set_text(md_snackbar.text, text);
    gtk_widget_show_all(md_snackbar.window);
}   

void hide_modal_snackbar(){
    gtk_widget_hide(md_snackbar.window);
} 

void show_modal_chat_context_menu(int top, int left){
    gtk_widget_show_all(md_chat_context_menu.window);
}

void hide_modal_chat_context_menu(){
    gtk_widget_hide(md_chat_context_menu.window);
} 
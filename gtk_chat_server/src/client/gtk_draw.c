#include "gtk_draw.h"
#include "gtk_callback.h"
#include "gtk_modal.h"

GtkWidget *main_window = NULL;
GtkWidget *page_stack = NULL;
GtkWidget* current_page = NULL;

LoginPage* login_page = NULL;
SignupPage* signup_page = NULL;
ServicePage* service_page = NULL;

const int WINDOW_MAX_WIDTH = 1000;
const int WINDOW_MAX_HEIGHT = 700;

#pragma region " GTK 위젯 생성 모듈화 "
void safe_label_set_text(GtkLabel *label, const gchar *text) {
    if (!label) return;

    // 1) NULL 처리
    if (!text) {
        gtk_label_set_text(label, "");
        return;
    }

    // 2) 깨진 UTF-8 은 U+FFFD 문자로 교체
    gchar *valid = g_utf8_make_valid(text, -1);

    // 3) 레이블에 설정 (내부적으로 Pango 레이아웃도 업데이트, 크기 재조정)
    gtk_label_set_text(label, valid);

    // 4) 꼭 해제
    g_free(valid);
}

GtkWidget* gtk_create_vbox(int width, int height, char* classname){
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(vbox, width, height);
    if(classname != NULL){
        gtk_style_class_toggle(vbox, classname, TRUE);
    }
    return vbox;
}

GtkWidget* gtk_create_hbox(int width, int height, char* classname){
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_size_request(hbox, width, height);
    if(classname != NULL){
        gtk_style_class_toggle(hbox, classname, TRUE);
    }
    return hbox;
}

GtkWidget* gtk_create_button(char* label, int width, int height, char* classname){

    GtkWidget* button = gtk_button_new_with_label(label);
    gtk_widget_set_size_request(button, width, height);
    if(classname != NULL){
        gtk_style_class_toggle(button, classname, TRUE);
    }
    return button;
}

GtkWidget* gtk_create_input(char* placeholder, int width, int height, char* classname){
    GtkWidget* entry = gtk_entry_new();
    gtk_widget_set_size_request(entry, width, height);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), placeholder);
    if(classname != NULL){
        gtk_style_class_toggle(entry, classname, TRUE);
    }
    return entry;
}

GtkWidget* gtk_create_eventbox(char* classname){
    GtkWidget* event_box = gtk_event_box_new();
    if(classname != NULL){
        gtk_style_class_toggle(event_box, classname, TRUE);
    }
    return event_box;
}

GtkWidget* gtk_create_label(char* label_text, int width, int height, char* classname){
    GtkWidget* label = gtk_label_new(NULL);
    gtk_widget_set_size_request(label, width, height);

    // PangoLayout *layout = gtk_label_get_layout(GTK_LABEL(label));
    safe_label_set_text(label, label_text);

    if(classname != NULL){
        gtk_style_class_toggle(label, classname, TRUE);
    }
    return label;
}

GtkWidget* gtk_create_scrolled_window(int width, int height, char* classname){
    GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, width, height);

    if(classname != NULL){
        gtk_style_class_toggle(scrolled_window, classname, TRUE);
    }
    return scrolled_window;
}
void gtk_align_center(GtkWidget* parent, GtkWidget* child){
    gtk_box_pack_start(parent, child, TRUE, FALSE, 0);
}

#pragma endregion

GtkWidget* gtk_box_add_item(GtkWidget* box, char* text){
    GtkWidget *label = gtk_label_new(text);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_widget_show(label);
    return label;
}


GtkWidget* gtk_create_login_page(){
    // page
    GtkWidget* page = gtk_create_vbox(WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT, "login_page");

    // header & body
    GtkWidget* header = gtk_create_hbox(WINDOW_MAX_WIDTH, 100, "header");
    GtkWidget* body = gtk_create_hbox(WINDOW_MAX_WIDTH, 600, "body");
    gtk_container_add(page, header);
    gtk_container_add(page, body);

    // main cont
    GtkWidget* cont = gtk_create_vbox(600, 600, "main_cont");
    gtk_align_center(body, cont);
    // gtk_container_add(body, cont);

    // title
    GtkWidget* title_box = gtk_create_vbox(600, 200, "title_box");
    gtk_container_add(cont, title_box);

    GtkWidget* title_text = gtk_create_label("TALKS",400, 100, "title_text");
    gtk_container_add(title_box, title_text);

    // input_section
    GtkWidget* input_section = gtk_create_vbox(400, 200, "input_section");
    gtk_box_pack_start(cont, input_section, TRUE, TRUE, 10);
    // gtk_container_add(cont, input_section);

    // input_boxes (w 400 h 200)
    GtkWidget* id_input_box = gtk_create_vbox(400, 100, "id_input_box");
    GtkWidget* pwd_input_box = gtk_create_vbox(400, 100, "pwd_input_box");
    gtk_container_add(input_section, id_input_box);
    gtk_container_add(input_section, pwd_input_box);

    // id inputs (w 400 h 100)
    GtkWidget* id_label = gtk_create_label("아이디",200, 50, "id_label");
    gtk_box_pack_start(id_input_box, id_label, FALSE,FALSE, 0);
    // gtk_container_add(id_input_box, id_label);
    login_page->id_input = gtk_create_input("아이디를 입력해주세요", 400, 50, "id_input");
    gtk_container_add(id_input_box, login_page->id_input);

    // pwd inputs (w 400 h 100)
    GtkWidget* pwd_label = gtk_create_label("비밀번호",200, 50, "pwd_label");
    gtk_box_pack_start(pwd_input_box, pwd_label, TRUE,FALSE, 0);

    // gtk_container_add(pwd_input_box, pwd_label);
    login_page->pwd_input = gtk_create_input("비밀번호를 입력해주세요", 400, 50, "pwd_input");
    gtk_container_add(pwd_input_box, login_page->pwd_input);

    // bottom_section
    GtkWidget* bottom_section = gtk_create_vbox(600, 200, "bottom_section");
    gtk_container_add(cont, bottom_section);

    // btn_login
    GtkWidget* btn_login = gtk_create_button("LOGIN",300, 100,  "btn_login");
    g_signal_connect(btn_login, "clicked", G_CALLBACK(g_callback_login), NULL);

    gtk_container_add(bottom_section, btn_login);

    // btn_signup
    GtkWidget* btn_signup = gtk_create_button("SIGN UP", 200, 50, "btn_signup");
    g_signal_connect(btn_signup, "clicked", G_CALLBACK(g_callback_move_signup_page), NULL);
    
    gtk_container_add(bottom_section, btn_signup);

    return page;
}

GtkWidget* gtk_create_signup_page(){
    // page
    GtkWidget* page = gtk_create_vbox(WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT, "signup_page");

    // header & body
    GtkWidget* header = gtk_create_hbox(WINDOW_MAX_WIDTH, 100, "header");
    GtkWidget* body = gtk_create_hbox(WINDOW_MAX_WIDTH, 600, "body");
    gtk_container_add(page, header);
    gtk_container_add(page, body);

    // main cont
    GtkWidget* cont = gtk_create_vbox(600, 600, "main_cont");
    gtk_align_center(body, cont);
    // gtk_container_add(body, cont);

    // // title
    GtkWidget* title_box = gtk_create_vbox(600, 200, "title_box");
    gtk_container_add(cont, title_box);

    GtkWidget* title_text = gtk_create_label("회원가입",400, 100, "title_text");
    gtk_container_add(title_box, title_text);

    // input_section
    GtkWidget* input_section = gtk_create_vbox(400, 300, "input_section");
    gtk_box_pack_start(cont, input_section, TRUE, TRUE, 10);
    // gtk_container_add(cont, input_section);

    // input_boxes (w 400 h 200)
    GtkWidget* id_input_box = gtk_create_vbox(400, 100, "id_input_box");
    GtkWidget* pwd_input_box = gtk_create_vbox(400, 100, "pwd_input_box");
    GtkWidget* name_input_box = gtk_create_vbox(400, 100, "name_input_box");
    gtk_container_add(input_section, id_input_box);
    gtk_container_add(input_section, pwd_input_box);
    gtk_container_add(input_section, name_input_box);

    // id inputs (w 400 h 100)
    GtkWidget* id_label = gtk_create_label("아이디",200, 50, "id_label");
    gtk_box_pack_start(id_input_box, id_label, FALSE,FALSE, 0);
    signup_page->id_input = gtk_create_input("아이디를 입력해주세요", 400, 50, "id_input");
    gtk_container_add(id_input_box, signup_page->id_input);

    // pwd inputs (w 400 h 100)
    GtkWidget* pwd_label = gtk_create_label("비밀번호",200, 50, "pwd_label");
    gtk_box_pack_start(pwd_input_box, pwd_label, TRUE,FALSE, 0);
    signup_page->pwd_input = gtk_create_input("비밀번호를 입력해주세요", 400, 50, "pwd_input");
    gtk_container_add(pwd_input_box, signup_page->pwd_input);

     // name inputs (w 400 h 100)
    GtkWidget* name_label = gtk_create_label("이름",200, 50, "name_label");
    gtk_box_pack_start(name_input_box, name_label, TRUE,FALSE, 0);
    signup_page->name_input = gtk_create_input("이름을 입력해주세요", 400, 50, "name_input");
    gtk_container_add(name_input_box, signup_page->name_input);


    // bottom_section
    GtkWidget* bottom_section = gtk_create_vbox(600, 100, "bottom_section");
    gtk_container_add(cont, bottom_section);

    // btn_signup
    GtkWidget* btn_signup = gtk_create_button("SIGN UP", 200, 50, "btn_signup");

    g_signal_connect(btn_signup, "clicked", G_CALLBACK(g_callback_signup), NULL);

    gtk_container_add(bottom_section, btn_signup);

    // btn_back
    GtkWidget* btn_back_login = gtk_create_button("LOGIN", 200, 50,  "btn_back_login");
    g_signal_connect(btn_back_login, "clicked", G_CALLBACK(g_callback_move_login_page), NULL);
    
    gtk_container_add(bottom_section, btn_back_login);

    return page;
}

GtkWidget* gtk_create_service_page(){
    GtkWidget* page = gtk_create_vbox(WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT, "service_page");

    // header & body
    GtkWidget* header = gtk_create_hbox(WINDOW_MAX_WIDTH, 50, "header");
    GtkWidget* body = gtk_create_hbox(WINDOW_MAX_WIDTH, 650, "body");
    gtk_container_add(page, header);
    gtk_container_add(page, body);

    // body (w 1000 h 650)
    GtkWidget* sidebar = gtk_create_vbox(100, 650, "sidebar");
    GtkWidget* main_cont = gtk_create_hbox(900, 650, "main_cont");
    gtk_container_add(body, sidebar);
    gtk_container_add(body, main_cont);

    // sidebar (w 100 h 650)
    GtkWidget* sidemenu_list = gtk_create_hbox(100, 600, "sidemenu_list");
    gtk_container_add(sidebar, sidemenu_list);

    // main_cont (w 900 h 650)
     GtkWidget* room_section = gtk_create_vbox(300, 650, "room_section");
    gtk_container_add(main_cont, room_section);
     GtkWidget* chat_section = gtk_create_vbox(600, 650, "chat_section");
    gtk_container_add(main_cont, chat_section);

    // room_section (w 300 h 650)
    GtkWidget* room_header = gtk_create_hbox(300, 50, "room_header");
    gtk_container_add(room_section, room_header);
    service_page->room_list_scroll = gtk_create_scrolled_window(300, 600, "room_list_scroll");
    gtk_container_add(room_section, service_page->room_list_scroll);

    GtkWidget* room_viewport = gtk_viewport_new(NULL, NULL);
    gtk_container_add(service_page->room_list_scroll, room_viewport);

    service_page->room_list = gtk_create_vbox(300, 600, "room_list");
    gtk_container_add(room_viewport, service_page->room_list);

    // room_header (w 300 h 600)
    service_page->inp_search_room = gtk_create_input("search room",250, 50, "inp_search_room");
    gtk_container_add(room_header, service_page->inp_search_room);
    service_page->btn_room_add = gtk_create_button("+", 50, 50, "btn_room_add");
    g_signal_connect(service_page->btn_room_add, "clicked", G_CALLBACK(g_callback_open_add_room), NULL);

    gtk_container_add(room_header, service_page->btn_room_add);

    // room_list

    // chat_section (w 600 h 650)
    GtkWidget* chat_header = gtk_create_hbox(300, 50, "chat_header");
    gtk_box_pack_start(chat_section, chat_header, TRUE, TRUE, 0);

    GtkWidget* chat_cont = gtk_create_vbox(300, 500, "chat_cont");
    gtk_container_add(chat_section, chat_cont);
    GtkWidget* chat_bottom = gtk_create_vbox(300, 100, "chat_bottom");
    gtk_box_pack_start(chat_section, chat_bottom, TRUE, FALSE, 0);

    // char_header
    service_page->inp_search_chat = gtk_create_input("search chat",300, 50, "inp_search_chat");
    gtk_container_add(chat_header, service_page->inp_search_chat);
    service_page->btn_room_menu = gtk_create_button("=", 250, 50, "btn_room_menu");
    gtk_container_add(chat_header, service_page->btn_room_menu);

    // chat_cont
    service_page->room_name_box = gtk_create_label("", 300, 50, "room_name_box");
    gtk_container_add(chat_cont, service_page->room_name_box);

    service_page->chat_notice = gtk_create_vbox(300, 50, "chat_notice");
    gtk_container_add(chat_cont, service_page->chat_notice);

    service_page->chat_list_scroll = gtk_create_scrolled_window(300, 400, "chat_list_scroll");
    gtk_container_add(chat_cont, service_page->chat_list_scroll);

    GtkWidget* chat_viewport = gtk_viewport_new(NULL, NULL);
    gtk_container_add(service_page->chat_list_scroll, chat_viewport);

    service_page->chat_list = gtk_create_vbox(300, 400, "chat_list");
    gtk_container_add(chat_viewport, service_page->chat_list);

    // chat_bottom (w 300 h 100)
    service_page->reply_box = gtk_create_vbox(250, 40, "reply_box");
    gtk_box_pack_start(chat_bottom, service_page->reply_box, TRUE, TRUE, 0);

    service_page->reply_user_id_label = gtk_create_label("",250, 20, "reply_user_id");
    gtk_box_pack_start(service_page->reply_box, service_page->reply_user_id_label, TRUE, TRUE, 0);

    service_page->reply_text_label = gtk_create_label("",250, 20, "reply_text");
    gtk_box_pack_start(service_page->reply_box, service_page->reply_text_label, TRUE, TRUE, 0);

    GtkWidget* chat_inp_hbox = gtk_create_hbox(250, 70, "chat_inp_box");
    gtk_container_add(chat_bottom, chat_inp_hbox);

    service_page->inp_chat = gtk_create_input("enter chat",240, 60, "inp_chat");
    gtk_box_pack_start(chat_inp_hbox, service_page->inp_chat, TRUE, TRUE, 0);

    service_page->btn_open_emoji_context = gtk_create_button("EMOJI",30, 60, "btn_open_emoji_context");
    g_signal_connect(service_page->btn_open_emoji_context, "clicked", G_CALLBACK(g_callback_show_emoji_context_menu), NULL);
    gtk_container_add(chat_inp_hbox, service_page->btn_open_emoji_context);

    GtkWidget* btn_chat_send = gtk_create_button("SEND",30, 60, "btn_chat_send");
    g_signal_connect(btn_chat_send, "clicked", G_CALLBACK(g_callback_send_chat), NULL);

    gtk_container_add(chat_inp_hbox, btn_chat_send);

    service_page->chat_list_size = 0;
    service_page->chat_item_list = NULL;
    service_page->room_list_size = 0;
    service_page->curr_reply_id = -1;

    return page;
}

void pages_init(){
    page_stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(page_stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(page_stack), 300);

    login_page = malloc(sizeof(LoginPage));
    signup_page = malloc(sizeof(SignupPage));
    service_page = malloc(sizeof(ServicePage));

    login_page->page = gtk_create_login_page();
    signup_page->page = gtk_create_signup_page();
    service_page->page = gtk_create_service_page();

    gtk_stack_add_titled(GTK_STACK(page_stack), login_page->page, "LOGIN", "LOGIN");
    gtk_stack_add_titled(GTK_STACK(page_stack), signup_page->page, "SIGNUP", "SINGUP");
    gtk_stack_add_titled(GTK_STACK(page_stack), service_page->page, "SERVICE", "SERVICE");

    gtk_container_add(main_window, page_stack);
}

// gtk 메인 위도우 생성 
void create_gtk_main_window()
{
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "TALKS"); // 메인 위도우 타이틀 (최상단 이름)
    gtk_window_set_default_size(GTK_WINDOW(main_window), WINDOW_MAX_WIDTH, WINDOW_MAX_HEIGHT); // 크기
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER); // 위치 

    gtk_widget_add_events(main_window, GDK_KEY_PRESS_MASK);

    g_signal_connect(main_window, "key-press-event", G_CALLBACK(on_key_press), main_window); // 키 이벤트 콜백

    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_exit), NULL); // 종료 콜백 

    build_layout();

    g_idle_add(on_realize, NULL);

    gtk_widget_show_all(main_window);

    gtk_main();
}

// css 
void css_init(){
    GtkCssProvider *main_prvd, *modal_prvd;
    GdkDisplay *display;
    GdkScreen  *screen;

    /* 1) 프로바이더 생성 */
    main_prvd  = gtk_css_provider_new();
    modal_prvd = gtk_css_provider_new();

    /* 2) CSS 로드(우선 로드한 뒤 add 하는 게 권장됩니다) */
    gtk_css_provider_load_from_path(main_prvd,
                                    "src/resources/style.css", NULL);
    gtk_css_provider_load_from_path(modal_prvd,
                                    "src/resources/modal.css", NULL);

    /* 3) 화면에 프로바이더 등록 */
    display = gdk_display_get_default();
    screen  = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(main_prvd),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(modal_prvd),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    /* 4) 프로바이더 해제 */
    g_object_unref(main_prvd);
    g_object_unref(modal_prvd);
}

void css_reload() {
    GtkCssProvider *css_provider;
    GdkDisplay *display;
    GdkScreen *screen;

    // 새로운 CSS Provider 생성
    css_provider = gtk_css_provider_new();

    // CSS 파일 경로를 다시 로드
    gtk_css_provider_load_from_path(css_provider, "src/resources/style.css", NULL);

    // 기본 화면을 가져오기
    display = gdk_display_get_default();
    screen = gdk_display_get_default_screen(display);

    // 기존의 스타일 제공자를 교체하여 새로운 CSS 파일 적용
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // CSS Provider 메모리 해제
    g_object_unref(css_provider);
}


void build_layout(){
    init_modal(main_window);
    // GtkWidget *vbox;

    // vbox = gtk_vbox_new(FALSE, 0);
    // gtk_style_class_toggle(vbox, "container", TRUE);
    
    // gtk_container_add(GTK_CONTAINER(window), vbox);
    pages_init();

    // gtk_container_add(GTK_CONTAINER(vbox), login_page);
    // current_page = login_page;
}

// css 토글하기 편하라고 만든 함수 
void gtk_style_class_toggle(GtkWidget* widget, char* classname, gboolean flag){
    if (!widget || !GTK_IS_WIDGET(widget) || !classname)
        return;
    GtkStyleContext *context = gtk_widget_get_style_context(widget);

    gchar *cls_copy = g_strdup(classname);
    if (!cls_copy)
        return; 

    if (flag == TRUE) {
        gtk_style_context_add_class(context, cls_copy);
    } else {
        gtk_style_context_remove_class(context, cls_copy);
    }
    g_free(cls_copy);
    gtk_widget_queue_draw(widget);
}

// 화면을 다시 그리는 것은 부하가 많음
// gtk가 여유로울 때 draw_ 함수를 호출하도록 함 
 gboolean _schedule_draw_room_list(gpointer user_data)
{
    draw_room_list();
    return G_SOURCE_REMOVE;  // 한 번만 실행
}

gboolean _schedule_draw_chat_list(gpointer user_data)
{
    draw_chat_list();
    return G_SOURCE_REMOVE;  // 한 번만 실행
}

GtkWidget* load_image(char* file_path, int size){
    GError *err = NULL;
    // 24×24px 크기로 로드
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(
        file_path,
        size, size,  // width, height
        &err
    );
    if (!pixbuf) {
        g_warning("이미지 로드 실패: %s", err->message);
        g_clear_error(&err);
        return NULL;
    }
    GtkWidget *img = gtk_image_new_from_pixbuf(pixbuf);
    // pixbuf 참조 카운트 감소
    g_object_unref(pixbuf);
    return img;
}

GtkWidget* get_emoji_img(char* emoji_name, int size){
    char file_path[64];
    snprintf(file_path, 64, "src/resources/images/emoji/%s", emoji_name);
    return load_image(file_path, size);
}

GtkWidget* get_react_icon_img(chat_react_type type, int size){
    char file_path[64];
    switch (type)
    {
        case LOVE:
            snprintf(file_path, 64, "%s", PATH_LOVE_ICON);
            break;
        case LIKE:
            snprintf(file_path, 64, "%s", PATH_LIKE_ICON);
            break;
        case CHECK:
        snprintf(file_path, 64, "%s", PATH_CHECK_ICON);
            break;
        case LAUGH:
        snprintf(file_path, 64, "%s", PATH_LAUGH_ICON);
            break;
        case SURPRISE:
        snprintf(file_path, 64, "%s", PATH_SURPRISE_ICON);
            break;
        case SAD:
        snprintf(file_path, 64, "%s", PATH_SAD_ICON);
            break;
        default:
            break;
    }
    return load_image(file_path, size);
}

// is_date_format = 0 : 시간 포맷 
char *format_kakao_style(char *datetime_str) {
    if(is_empty_string(datetime_str) == SUCCESS){
        return "";
    }
    struct tm tm_input = {0};
    strptime(datetime_str, "%Y-%m-%d %H:%M:%S", &tm_input);
    time_t input_time = mktime(&tm_input);

    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);

    // 오늘 날짜 00:00 기준 시간 계산
    struct tm tm_midnight = tm_now;
    tm_midnight.tm_hour = 0;
    tm_midnight.tm_min = 0;
    tm_midnight.tm_sec = 0;
    time_t midnight = mktime(&tm_midnight);

    char *result = malloc(64);
    if (!result) return NULL;

    if (difftime(input_time, midnight) >= 0) {
        // 오늘이면 시간만 표시 (오전/오후 h:mm)
        int hour = tm_input.tm_hour;
        int minute = tm_input.tm_min;
        const char *ampm = (hour >= 12) ? "오후" : "오전";
        if (hour == 0) hour = 12;
        else if (hour > 12) hour -= 12;

        snprintf(result, 64, "%s %d:%02d", ampm, hour, minute);
    } else {
        // 이전이면 날짜 표시
        const char *weekday[] = {
            "일요일", "월요일", "화요일", "수요일",
            "목요일", "금요일", "토요일"
        };
        snprintf(result, 64, "%d년 %d월 %d일 %s",
                 tm_input.tm_year + 1900,
                 tm_input.tm_mon + 1,
                 tm_input.tm_mday,
                 weekday[tm_input.tm_wday]);
    }

    return result;
}
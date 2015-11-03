#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <jack/jack.h>

jack_client_t* client;
const char** ports = 0;
const char** portsOut = 0;
int n_ports = 0;
int n_out_ports = 0;

int line = 0;

void show_connections(const char* type)
{
    char* t = 0;
    if( strcmp( JACK_DEFAULT_AUDIO_TYPE, type ) == 0 )
        t = "Audio";
    else
        t = "MIDI";
    mvprintw(line++, 0, "JACK %s Connections:\n", t);

    for(int i = 0; ports[i] != 0; i++ ) {
        jack_port_t *p = jack_port_by_name(client, ports[i]);
        if( !strcmp(type, jack_port_type(p) ) == 0 ) {
            continue;
        }
        const char** connect = jack_port_get_all_connections( client, p );

        if( connect ) {
            for(int o = 0; connect[o] != 0; o++) {
                mvprintw(line++, 0, "%s -> %s\n", connect[o], ports[i]);
                //mvprintw(30, 10, "connected %s -> %s\n", connect[o], ports[i] );
            }
            jack_free(connect);
        }
    }
}

void list_ports(const char* type, int* sel)
{
    int left  = 20;
    if( line > 18 )
        left = line + 2;
    int arrow = left;
    int right = 20;
    for(int i = 0; portsOut[i] != 0; i++ ) {
        jack_port_t *p = jack_port_by_name(client, portsOut[i]);
        mvprintw(left , 0, "   %s", portsOut[i]);
        mvprintw(arrow+sel[0], 0, "=>");
        left++;
    }
    left = arrow;
    for(int i = 0; ports[i] != 0; i++ ) {
        jack_port_t *p = jack_port_by_name(client, ports[i]);
        mvprintw(left , 30, "   %s", ports[i]);
        mvprintw(arrow+sel[1], 30, "=>");
        left++;
    }
}

int main()
{
    client = jack_client_open("CliConnect",
                              JackNoStartServer, 0 );
    if( !client ) {
        printf("JACK not running - exit.\n");
        exit(0);
    }
    ports = jack_get_ports( client, 0, 0, JackPortIsInput );
    for(n_ports = 0; ports[n_ports] != 0; n_ports++) {}
    portsOut = jack_get_ports( client, 0, 0, JackPortIsOutput );
    for(n_out_ports = 0; portsOut[n_out_ports] != 0; n_out_ports++) {}


    // Start the UI
    initscr();
    clear();
    noecho();
    curs_set(0);
    cbreak();
    // 2 part array, selection left, selection right
    int  selected[2];
    memset(selected, 0, sizeof(int)*2);
    bool onRight = 0;

    int row, col;
    getmaxyx( stdscr, row, col );
    int row_old, col_old;

    while(1) {
        getmaxyx( stdscr, row, col );
        if(row != row_old || col != col_old) {
            row_old = row;
            col_old = col;
            clear();
        }
        refresh();

        line = 0;
        show_connections( JACK_DEFAULT_AUDIO_TYPE );
        line++;
        show_connections( JACK_DEFAULT_MIDI_TYPE );
        line++;
        list_ports(JACK_DEFAULT_AUDIO_TYPE, selected);
        refresh();

        char c = getch();
        if( c == 'q' || c == 'Q' )
            break;
        if( c == 'j' ) {
            if( onRight ) { // input / writeable / ports
                selected[onRight]++;
                if( selected[onRight] >= n_ports )
                    selected[onRight] = 0;
            } else {
                selected[onRight]++;
                if( selected[onRight] >= n_out_ports )
                    selected[onRight] = 0;
            }
        }
        if( c == 'k' ) {
            if(onRight) {
                selected[onRight]--;
                if( selected[onRight] < 0 )
                    selected[onRight] = n_ports-1;
            } else {
                selected[onRight]--;
                if( selected[onRight] < 0 )
                    selected[onRight] = n_out_ports-1;
            }
        }
        if( c == 'l' || c == 'h' )
            onRight = !onRight;

        if( c == ' ' ) {
            int r = jack_connect( client, portsOut[selected[0]],
                                  ports[selected[1]] );
            if( r )
                jack_disconnect( client, portsOut[selected[0]],
                                 ports[selected[1]] );
            clear();
        }

        if( c == 'r' ) {
            jack_free( ports );
            jack_free( portsOut );
            ports = jack_get_ports( client, 0, 0, JackPortIsInput );
            portsOut = jack_get_ports( client, 0, 0, JackPortIsOutput );
            for(n_ports = 0; ports[n_ports] != 0; n_ports++) {}
            for(n_out_ports = 0; ports[n_out_ports] != 0; n_out_ports++) {}
        }
    }

    endwin();

    jack_free( ports );
    jack_free( portsOut );
    jack_client_close( client );
    return 0;
}
#include "game.h"

static scm_t_bits game_tag;

static Game*
check_game (SCM game_smob)
{
    scm_assert_smob_type (game_tag, game_smob);
     
    return (Game *) SCM_SMOB_DATA (game_smob);
}

static SCM
make_game ()
{
    SCM smob;
    Game *game;

    /* Step 1: Allocate the memory block.
     */
    game = (Game *) scm_gc_malloc (sizeof (Game), "game");

    /* Step 2: Initialize it with straight code.
     */
    game->on_start = SCM_BOOL_F;
    game->on_update = SCM_BOOL_F;
    game->on_draw = SCM_BOOL_F;
     
    /* Step 3: Create the smob.
     */
    SCM_NEWSMOB (smob, game_tag, game);

    return smob;
}

static SCM
on_start_hook (SCM game_smob, SCM callback)
{
    Game *game = check_game(game_smob);
    
    game->on_start = callback;

    return SCM_UNSPECIFIED;
}

static SCM
on_update_hook (SCM game_smob, SCM callback)
{
    Game *game = check_game(game_smob);
    
    game->on_update = callback;

    return SCM_UNSPECIFIED;
}

static SCM
on_draw_hook (SCM game_smob, SCM callback)
{
    Game *game = check_game(game_smob);
    
    game->on_draw = callback;

    return SCM_UNSPECIFIED;
}

static SCM
game_loop(SCM game_smob) {
    Game *game = check_game(game_smob);
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_FONT *font = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    const float timestep = 1.0 / 60.0;
    float last_time = 0;
    float fpsbank = 0;
    char fps_string[16] = "0 fps";
    int frames = 0;
    bool running = true;
    bool redraw = true;

    srand (time (NULL));

    if(!al_init())
    {
	fprintf (stderr, "failed to initialize allegro!\n");
    }

    if (!al_init_image_addon ())
    {
	fprintf (stderr, "failed to initialize image addon!\n");
    }

    al_init_font_addon ();
    al_init_ttf_addon ();

    if(!al_install_keyboard ()) {
	fprintf (stderr, "failed to initialize keyboard!\n");
    }

    display = al_create_display (800, 600);
    if (!display) {
	fprintf (stderr, "failed to create display!\n");
    }

    font = al_load_ttf_font ("CarroisGothic-Regular.ttf", 24, 0);

    if (scm_is_true (game->on_start))
	scm_call_0 (game->on_start);

    timer = al_create_timer (timestep);
    event_queue = al_create_event_queue ();
    al_register_event_source (event_queue, al_get_display_event_source (display));
    al_register_event_source (event_queue, al_get_timer_event_source (timer));
    al_register_event_source (event_queue, al_get_keyboard_event_source ());

    al_start_timer(timer);
    last_time = al_get_time();

    while(running) {
	// Handle events
	ALLEGRO_EVENT event;
				
	al_wait_for_event(event_queue, &event);
				
	if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
	    running = false;
	}
	else if(event.type == ALLEGRO_EVENT_TIMER) {
	    redraw = true;

	    // FPS
	    float time = al_get_time();
	    fpsbank += time - last_time;
	    last_time = time;
	    if(fpsbank >= 1) {
		sprintf(fps_string, "%d fps", frames);
		frames = 0;
		fpsbank -= 1;
	    }

	    if (scm_is_true (game->on_update))
		scm_call_1 (game->on_update, scm_from_double(timestep));
	}
	else if(event.type == ALLEGRO_EVENT_KEY_UP) {
	    if(event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
		running = false;
	    }
	}

	// Draw
	if(redraw && al_is_event_queue_empty(event_queue)) {
	    redraw = false;
	    frames += 1;
	    al_clear_to_color(al_map_rgb(0,0,0));

	    if (scm_is_true (game->on_draw))
		scm_call_0 (game->on_draw);

	    al_draw_text(font, al_map_rgb(255, 255, 255), 790, 570, ALLEGRO_ALIGN_RIGHT, fps_string);

	    al_flip_display();
	}
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return SCM_UNSPECIFIED;
}

static SCM
mark_game (SCM game_smob)
{
    Game *game = (Game *) SCM_SMOB_DATA (game_smob);
    
    // Mark callbacks
    scm_gc_mark (game->on_start);
    scm_gc_mark (game->on_update);

    return game->on_draw;
}

static size_t
free_game (SCM game_smob)
{
    Game *game = (Game *) SCM_SMOB_DATA (game_smob);
     
    scm_gc_free (game, sizeof (Game), "game");
     
    return 0;
}
     
static int
print_game (SCM game_smob, SCM port, scm_print_state *pstate)
{
    //Game *game = (Game *) SCM_SMOB_DATA (game_smob);
     
    scm_puts ("#<Game>", port);
     
    /* non-zero means success */
    return 1;
}

void
init_game_type (void)
{
    game_tag = scm_make_smob_type ("Game", sizeof (Game));
    scm_set_smob_mark (game_tag, mark_game);
    scm_set_smob_free (game_tag, free_game);
    scm_set_smob_print (game_tag, print_game);

    scm_c_define_gsubr ("make-game", 0, 0, 0, make_game);
    scm_c_define_gsubr ("game-on-start-hook", 2, 0, 0, on_start_hook);
    scm_c_define_gsubr ("game-on-update-hook", 2, 0, 0, on_update_hook);
    scm_c_define_gsubr ("game-on-draw-hook", 2, 0, 0, on_draw_hook);
    scm_c_define_gsubr ("game-run", 1, 0, 0, game_loop);
}

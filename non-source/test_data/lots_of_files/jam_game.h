/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 */

struct Memory_Block{
	void *base;
	i32 size, cursor;
};

struct Track{
    Sound sound;
    Track *next_free;
    i32 sample_pos;
    real32 sample_pos_dec;
    real32 volume;
    real32 bend;
    bool32 playing;
    bool32 looping;
    i32 frames_delay;
};

enum Block_Type{
    ERROR,
    ZOMBIE,
    HUMAN,
    BRAIN,
    AMMO,
    BOMB,
    WALL,
    BLOCK_TYPE_COUNT
};

#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3

#define ZOMBIE_TURN_THRESHOLD 360

Block_Type block_freq_table[] = {
    ZOMBIE,
    ZOMBIE,
    ZOMBIE,
    ZOMBIE,
    HUMAN,
    HUMAN,
    HUMAN,
    HUMAN,
    HUMAN,
    HUMAN,
    BRAIN,
    BRAIN,
    BRAIN,
    AMMO,
    AMMO,
    AMMO,
    AMMO,
    AMMO,
    BOMB,
    BOMB,
    BOMB,
    WALL,
    WALL,
    WALL,
    WALL,
    WALL
};

struct Entity{
    Block_Type type;
    i32 grid_x, grid_y;
    real32 show_x, show_y;
    bool32 active;
    i32 facing;
    bool32 firing;
    i32 step_forward;
    bool32 death_marked;
    i32 infection_amount;
    real32 wobble;
};

struct Spawn_Request{
    Block_Type type;
    i32 x, y;
    bool32 random;
};

#define TEST_ORDER 0

struct Level_Data{
    i32 min_score;
    i32 fall_tick_time;
};

Level_Data levels[] = {
    {0, 30},
    {400, 26},
    {1200, 22},
    {2400, 18},
    {3600, 15}
};

struct Particle{
    i32 tex_index;
    Vec2 pos, vel;
    i32 life_counter;
    real32 rot;
    real32 rot_vel;
};

#define LIST_PRT_TYPES(m)                         \
    m(prt_ammo);                                  \
    m(prt_blood);                                 \
    m(prt_bone);                                  \
    m(prt_brain);                                 \
    m(prt_human);                                 \
    m(prt_muzzle);                                \
    m(prt_tnt);                                   \
    m(prt_wall);                                  \
    m(prt_zombie);                                \
    m(prt_bullet)

struct Particle_Textures{
    Render_Texture prt_ammo[5];
    Render_Texture prt_blood[6];
    Render_Texture prt_bone[6];
    Render_Texture prt_brain[4];
    Render_Texture prt_human[4];
    Render_Texture prt_muzzle[2];
    Render_Texture prt_tnt[5];
    Render_Texture prt_wall[5];
    Render_Texture prt_zombie[4];
    Render_Texture prt_bullet[1];
};

struct Particle_Type{
    i32 tex_count;
    i32 first_tex_id;
};

struct App_Vars{
	Memory_Block block;
	Game_Input prev_input;
    GJ_Context gj;
    i32 table_id;
    pcg32_random rnd;
    
    Render_Texture background;
    Render_Texture human[1][4];
    Render_Texture zombie[1];
    Render_Texture ammo;
    Render_Texture brain;
    Render_Texture bomb;
    Render_Texture wall;
    
    Render_Texture *prt_textures;
    Particle_Type *prt_types;
    i32 prt_type_count;
    
    Particle particles[128];
    i32 particle_count, particle_max;
    
#define DECL_INT(t) i32 t##_index
    LIST_PRT_TYPES(DECL_INT);
#undef DECL_INT
    
    Render_Texture scorename;
    Render_Texture scoreback;
    Render_Texture gameover;
    Render_Texture shadow;
    
    Render_Texture finish_button[2];
    
    Render_Texture overreact;
    Render_Texture title;
    Render_Texture title_button;
    Render_Texture controls;
    
	Font font;
    Font small_font;
    
    Track music;
    Track sfx_tracks[32];
    Track *sfx_free;
    
    Sound ammo_flip1;
    Sound ammo_flip2;
    Sound ammo_land;
    Sound brains1;
    Sound brains2;
    Sound explosion;
    Sound gun_shot;
    Sound person_flip1;
    Sound person_flip2;
    Sound person_land;
    Sound reload;
    Sound soft_flip1;
    Sound soft_flip2;
    Sound soft_land1;
    Sound soft_land2;
    Sound wall_flip1;
    Sound wall_flip2;
    Sound wall_land;
    Sound zombie_break;
    Sound splat_death;
    Sound score1;
    Sound score2;
    
    Sound menu_music;
    Sound gameplay1;
    Sound gameplay2;
    
    bool32 song_done;
    
    i32 silence_timer;
    i32 fall_timer;
    i32 refall_timer;
    bool32 game_over;
    i32 title_screen;
    bool32 controls_screen;
    
    bool32 need_new_block;
    bool32 need_to_fill_gaps;
    bool32 chain_reacting;
    
    i32 level;
    
#define COMBO_TIME 20
    
    i32 reaction_timer;
    i32 fill_timer;
    
    i32 score;
    i32 nonsense_score;
    
    i32 active_field;
    i32 blink_timer;
    
    char user_name[16];
    i32 user_name_len;
    
    char user_token[16];
    i32 user_token_len;
    
    char teaser_name[32];
    char teaser_score[64];
    i32 teaser_name_len;
    i32 teaser_score_len;
    bool32 looked_up_teaser;
    
#define WIDTH 6
#define HEIGHT 7
    
    Entity entities[WIDTH*HEIGHT];
    i32 entity_count, entity_max;
    
    Spawn_Request spawns[16];
    i32 spawn_count, spawn_max;
    
    Entity *to_free[16];
    i32 free_count, free_max;
    
    Entity *grid[WIDTH*HEIGHT];
    
#if TEST_ORDER == 1
    i32 test_type_i;
#endif
};

Grid_Pos right_shot[] = {
    {1, 0},
    {2, 0},
    {3, 0}
};

Grid_Pos up_shot[] = {
    {0, -1},
    {0, -2},
    {0, -3}
};

Grid_Pos left_shot[] = {
    {-1, 0},
    {-2, 0},
    {-3, 0}
};

Grid_Pos down_shot[] = {
    {0, 1},
    {0, 2},
    {0, 3}
};


/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Game to Platform Layer.
 */

struct Game_Memory{
	void *data;
	i32 size;
};

struct Game_Step_Target{
	Game_Render_Target render;
	
	bool32 can_fullscreen;
	bool32 *is_fullscreen;
	
	real32 second_per_frame;
	i32 audio_samples_per_second;
    
    char btn1, btn2;
};

struct Game_Input{
	struct{
		real32 stick_x, stick_y;
	} analog;
	struct{
		bool8 up, down, left, right;
	} digital;
	bool8 button[8];
	bool8 pause;
	bool8 back;
	
	bool8 is_analog;
	
	bool8 toggle_edit_mode;
    
    bool8 key_input;
    char key_code;
};

struct Game_Audio_Target{
	i16 *samples;
    i32 count_per_channel;
    i32 channels;
	i32 samples_per_second;
	real32 second_per_frame;
};

#define SIG_APP_FILL_SAMPLES(name) void(name)(Game_Audio_Target target, Game_Memory memory)
#define SIG_APP_STEP(name) bool32(name)(Game_Step_Target target, Game_Input input, Game_Memory memory, bool32 first)

internal
SIG_APP_FILL_SAMPLES(game_fill_samples);

internal
SIG_APP_STEP(game_step);


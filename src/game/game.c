/// Game scene (source)
/// (c) 2017 Jani Nykänen

#include "game.h"

#include "../engine/app.h"
#include "../engine/graphics.h"
#include "../engine/bitmap.h"
#include "../engine/controls.h"
#include "../engine/assets.h"
#include "../engine/transform.h"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"

#include "stage.h"
#include "player.h"
#include "slime.h"
#include "crystal.h"
#include "blood.h"

/// Bitmap font
static BITMAP* bmpFont;

/// Kills
static int kills;
/// Percentage
static int percentage;

/// Player
static PLAYER player;

/// Bullet count
#define BULLET_COUNT 32
/// Bullets
static BULLET bullets[BULLET_COUNT];
/// Slime count
#define SLIME_COUNT 32
/// Slimes
static SLIME slimes[32];
/// Slime timer count
#define SLIME_TIMER_COUNT 4
/// Slime timers
static float slimeTimer[SLIME_TIMER_COUNT];
/// Crystal count
#define CRYSTAL_COUNT 32
/// Crystals
static CRYSTAL crystals[32];
/// Bomb count
static int bombCount;
/// Blood count
#define BLOOD_COUNT 128
/// Blood
static BLOOD blood[BLOOD_COUNT];

/// Draw HUD
static void draw_hud()
{
    // Draw percentage
    int hundreds = (int)floor(percentage / 10);
    int decimals = percentage - hundreds*10;

    char prcStr[32];
    snprintf(prcStr,32,"%d.%d ",hundreds,decimals);
    int dpos = (int)floor(hundreds/100) > 0 ? 5 : ( (int)floor(hundreds/10) > 0 ? 4 : 3);
    prcStr[dpos] = '%';

    draw_text(bmpFont,(Uint8*)prcStr,dpos+1,64,1,-1,0,true);

    // Draw kills
    char num[3];
    snprintf(num,3,"%d",kills);

    Uint8 kills[32];
    kills[0] = 2;
    kills[1] = 3;
    kills[2] = 4;
    kills[3] = num[0];
    kills[4] = num[1];
    kills[5] = 0;
    
    draw_text(bmpFont,kills,32,2,1,-1,0,false);

    // Draw crystals
    snprintf(num,3,"%d",player.crystals);

    Uint8 crystals[32];
    crystals[0] = 5;
    crystals[1] = 6;
    crystals[2] = 4;
    crystals[3] = num[0];
    crystals[4] = num[1];
    crystals[5] = 0;
    
    draw_text(bmpFont,crystals,32,88,1,-1,0,false);
}

/// Push slime to the screen
/// < index Timer index
static void push_slime(int index)
{
    SLIME * s = &slimes[0];
    int i = 0;
    for(; i < SLIME_COUNT; i++)
    {
        if(slimes[i].dead == true && slimes[i].dying == false)
        {
            s = &slimes[i];
            break;
        }
    }

    int id =  rand() % 8 + index;
    if(id >= 8) id -= 8;

    bombCount --;
    if(bombCount <= 0)
    {
        id = rand() %2 + 8;
        bombCount = rand() % 6 + 4;
    }

    VEC2 pos;
    switch(id)
    {
    case 8:
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        
        pos.x = 128+8;
        pos.y = 96-12;
        
        break;

    case 9:
    case 6:
    case 5:
        pos.x = 128+8;
        pos.y = 16 + (float)(rand() % 56) - 8;
        break;

    case 7:
        pos.x = 128+8;
        pos.y = -8;
        break;

    default:
        break;
    }
    put_slime(s,pos,id);
}

/// Init game
static int game_init()
{
    srand(time(NULL));

    bmpFont = get_bitmap("font");

    init_stage();

    percentage = 0;
    kills = 0;

    // Create objects
    player = create_player();
    int i = 0;
    for(; i < BULLET_COUNT; i++)
    {
        bullets[i] = create_bullet();
    }
    for(i=0; i < SLIME_COUNT; i++)
    {
        slimes[i] = create_slime();
    }
    for(i=0; i < SLIME_TIMER_COUNT; i++)
    {
        slimeTimer[i] = (float) (rand() % 60 + 30) + i*15;
    }
    for(i=0; i < CRYSTAL_COUNT; i++)
    {
        crystals[i] = create_crystal();
    }
    for(i=0; i < BLOOD_COUNT; i++)
    {
        blood[i] = create_blood();
    }

    bombCount = rand() % 4 + 4;

    return 0;
}

/// Update game
/// tm Time multiplier
static void game_update(float tm)
{
    // Update stage
    update_stage(tm);
    // Update bullets
    int i = 0;
    int i2 = 0;
    for(; i < BULLET_COUNT; i++)
    {
        bullet_update(&bullets[i],tm);
    }
    // Update slimes
    for(i=0; i < SLIME_COUNT; i++)
    {
        slime_update(&slimes[i],tm);
        slime_collision(&slimes[i],&player,bullets,BULLET_COUNT);
        for(i2 = 0; i2 < SLIME_COUNT; i2++)
        {
            if(i == i2) continue;
            slime_to_slime_collision(&slimes[i],&slimes[i2]);
        }
    }
    // Update crystals
    for(i=0; i < CRYSTAL_COUNT; i++)
    {
        crystal_update(&crystals[i],&player,tm);
    }
    // Update player
    pl_update(&player,tm);

    // Update blood
    for(i=0; i < BLOOD_COUNT; i++)
    {
        blood_update(&blood[i],tm);
    }

    // Update timers
    float speed = get_global_speed();
    for(i=0; i < SLIME_TIMER_COUNT; i++)
    {
        slimeTimer[i] -= 1.0f * speed * tm;
        if(slimeTimer[i] <= 0.0f)
        {
            push_slime(i);
            slimeTimer[i] = (float)(rand() % 90 + 40);
        }
    }

}

/// Draw game
void game_draw()
{
    clear_frame(0b00101010);

    set_translation(0,0);

    int i = 0;
    // Draw stage
    draw_stage();
    // Draw slimes
    for(i=0; i < SLIME_COUNT; i++)
    {
        slime_draw(&slimes[i]);
    }
    // Post-draw slimes
    for(i=0; i < SLIME_COUNT; i++)
    {
        slime_post_draw(&slimes[i]);
    }
    // Draw blood
    for(i=0; i < BLOOD_COUNT; i++)
    {
        blood_draw(&blood[i]);
    }
    // Draw crystals
    for(i=0; i < CRYSTAL_COUNT; i++)
    {
        crystal_draw(&crystals[i]);
    }

    // Draw player
    pl_draw(&player);
    // Draw bullets
    for(i=0; i < BULLET_COUNT; i++)
    {
        bullet_draw(&bullets[i]);
    }
    // Draw HUD
    draw_hud();
}

/// Destroy game
static void game_destroy()
{
    destroy_assets();
}

/// Get the next nonexistent bullet in the array
BULLET* get_next_bullet()
{
    int i = 0;
    for(; i < BULLET_COUNT; i++)
    {
        if(bullets[i].exist == false && bullets[i].dying == false)
            return &bullets[i];
    }
    return &bullets[0];
}

/// Create crystals
void create_crystals(VEC2 pos, int count)
{
    VEC2 speed;

    int loop = 0;
    int i;
    for(; loop < count; loop++)
    {
        for(i=0; i < CRYSTAL_COUNT; i++)
        {
            if(crystals[i].exist || crystals[i].vanishing) continue;

            speed.x = -0.5f + (float)(rand() % 200) / 100.0f;
            speed.y =  0.25f -(float)(rand() % 200 ) / 100.0f;

            put_crystal(&crystals[i],pos,speed);


            break;
        }
    }
}

/// Create some nasty blood
void create_blood_effect(VEC2 pos, int amount)
{
    VEC2 speed;

    int loop = 0;
    int i;
    for(; loop < amount; loop++)
    {
        for(i=0; i < BLOOD_COUNT; i++)
        {
            if(blood[i].exist) continue;

            speed.x = -1.5f + (float)(rand() % 300) / 100.0f;
            speed.y = 0.5f - (float)(rand() % 300 ) / 100.0f;

            put_blood(&blood[i],pos,speed);

            break;
        }
    }
}

/// Add percentage
void add_percentage(int amount)
{
    percentage += amount;
    if(percentage >= 1000)
    {
        percentage = 1000;
        // Trigger final boss event
    }
}

/// Get game scene
SCENE get_game_scene()
{
    // Set scene functions
    SCENE s = (SCENE){game_init,game_update,game_draw,game_destroy};

    // Set scene name
    set_scene_name(&s,"game");

    return s;
}

#pragma bank 5

#include "states/Shmup.h"
#include "Actor.h"
#include "Camera.h"
#include "Collision.h"
#include "DataManager.h"
#include "GameTime.h"
#include "Input.h"
#include "ScriptRunner.h"
#include "Trigger.h"
#include "data_ptrs.h"

#define SHOOTER_HURT_IFRAMES 10


UBYTE shooter_push = 0;
UBYTE shooter_push_strength = 0;
UBYTE shooter_diag_tracker = 0;
UBYTE shooter_collision_off_timer = 0;
UBYTE shooter_collision_off = FALSE;
Vector2D shooter_move;
UBYTE shooter_old_facing = 1;


void Start_Shmup() {
  camera_offset.x = 0;
  camera_offset.y = 0;
  camera_deadzone.x = 0;
  camera_deadzone.y = 0;
  shooter_move.x = 0;
  shooter_move.y = 0;
  camera_dest.x = camera_pos.x;
  camera_dest.y = camera_pos.y;

  player.animate = TRUE;
}

void Update_Shmup() {
	UBYTE tile_x, tile_y, hit_actor;

	tile_x = DIV_8(player.pos.x);
	tile_y = DIV_8(player.pos.y);

	// Check for trigger collisions
	if (ActivateTriggerAt(tile_x, tile_y, FALSE)) {
		return;
	}
	
	if(free_range_mode == 0)
	{
		// Determines how quickly to push the player.
		if(camera_settings != CAMERA_LOCK_FLAG)
		{
			if (camera_speed == 0)
				{shooter_push = 4;}
			else if (camera_speed == 1 && IS_FRAME_2)
				{shooter_push = 4;}
			else if(camera_speed == 3 && IS_FRAME_4)
				{shooter_push = 4;}
			else if (camera_speed == 7 && IS_FRAME_4)
				{shooter_push += 2;}
			else if (camera_speed == 15 && IS_FRAME_4)
				{shooter_push += 1;}
		}
		
		if(shooter_push >= 4)
		{
			if(camera_pos.x < camera_dest.x)
			{
				player.pos.x ++;
			}
			else if(camera_pos.x > camera_dest.x)
			{
				player.pos.x --;
			}
			
			if(camera_pos.y < camera_dest.y)
			{
				player.pos.y ++;
			}
			else if(camera_pos.y > camera_dest.y)
			{
				player.pos.y --;
			}
			shooter_push -= 4;
		}
		
		// Checks to see if the player has been pushed into a surface via camera movement.
		shooter_impact = 0;
		if(TileAt(tile_x + 1,tile_y))
		{
			shooter_impact = 1;
		}
	}

    // Check input to set player movement
	// Vertical movement
	shooter_move.y = 0;
	if (INPUT_UP) 
	{
		if (free_range_mode == 0 && player.pos.y - 6 > camera_pos.y - SCREEN_HEIGHT_HALF)
			{shooter_move.y = -1;}
		else if (free_range_mode == 1 && !TileAt(tile_x, tile_y - 1))
			{shooter_move.y = -1;}
		else 
			{shooter_move.y = 0;}
	}
	else if (INPUT_DOWN) 
	{
		if (free_range_mode == 0 && player.pos.y + 8 < camera_pos.y + SCREEN_HEIGHT_HALF - 15)
			{shooter_move.y = 1;}
		else if (free_range_mode == 1 && !TileAt(tile_x, tile_y + 1))
			{shooter_move.y = 1;}
		else 
			{shooter_move.y = 0;}
	}
	
	//For a two-line UI HUD (which is what it's formatted for now):
		//if (player.pos.y + 8 < camera_pos.y + SCREEN_HEIGHT_HALF - 31)
	//For a three-line UI HUD:
		//if (player.pos.y + 8 < camera_pos.y + SCREEN_HEIGHT_HALF - 39)
	//For no UI HUD:
		//if (player.pos.y + 8 < camera_pos.y + SCREEN_HEIGHT_HALF + 1)
	
	// Horizontal movement
	shooter_move.x = 0;
	if (INPUT_LEFT) 
	{
		if (free_range_mode == 0 && player.pos.x > camera_pos.x - SCREEN_WIDTH_HALF)
			{shooter_move.x = -1;}
		else if (free_range_mode == 1 && !TileAt(tile_x, tile_y))
			{shooter_move.x = -1;}
		else 
			{shooter_move.x = 0;}
		
		if (free_range_mode == 1)
		{
			shooter_facing = 0;
		}
	}
	else if (INPUT_RIGHT) 
	{
		if (free_range_mode == 0 && player.pos.x < camera_pos.x + SCREEN_WIDTH_HALF - 14)
			{shooter_move.x = 1;}
		else if (free_range_mode == 1 && !TileAt(tile_x + 2, tile_y))
			{shooter_move.x = 1;}
		else {shooter_move.x = 0;}
		
		if (free_range_mode == 1)
		{
			shooter_facing = 1;
		}
	}

    // Move player
	if (player.move_speed == 0) 
	{
		// Half speed only move every other frame
		if (IS_FRAME_2) {
		  if (shooter_move.x != 0 && shooter_move.y != 0)
		  {
			if (IS_FRAME_4)
			{
				player.pos.y += (WORD)shooter_move.y;
				player.pos.x += (WORD)shooter_move.x;
			}
		  }
		  else
		  {
			player.pos.y += (WORD)shooter_move.y;
			player.pos.x += (WORD)shooter_move.x;
		  }
		}
	} 
	else 
	{
		if (shooter_move.x != 0 && shooter_move.y != 0)
		  {
				// Slows the player's movement down to 80% speed using subpixel movement if
				// the player is moving both veritically and horizontally
				shooter_diag_tracker += 8;
				if(shooter_diag_tracker >= 10)
				{
					player.pos.y += (WORD)shooter_move.y * player.move_speed;
					player.pos.x += (WORD)shooter_move.x * player.move_speed;
					shooter_diag_tracker -= 10;
				}
		  }
		  else
		  {
			player.pos.y += (WORD)shooter_move.y * player.move_speed;
			player.pos.x += (WORD)shooter_move.x * player.move_speed;
		  }
	}
	
	//Handles sprite and damage animation.
	shooter_flash_timer = CLAMP(shooter_flash_timer -1, 0, 300);
	if(shooter_facing == 0)
	{
		if(shooter_flash_timer > 1 && IS_FRAME_4)
		{
			player.dir.y = 1;
			player.dir.x = -1;
			player.rerender = TRUE;
		}
		else if(shooter_flash_timer > 1 && !IS_FRAME_4)
		{
			player.dir.y = 0;
			player.dir.x = -1;
			player.rerender = TRUE;
		}
		else if(shooter_flash_timer == 1)
		{
			player.dir.x = -1;
			player.dir.y = 0;
			player.rerender = TRUE;
		}
		else
		{
			player.dir.x = -1;
			player.dir.y = 0;
			player.rerender = TRUE;
		}
	}
	else if(shooter_facing == 1)
	{
		if(shooter_flash_timer > 1 && IS_FRAME_4)
		{
			player.dir.y = -1;
			player.dir.x = 0;
			player.rerender = TRUE;
		}
		else if(shooter_flash_timer > 1 && !IS_FRAME_4)
		{
			player.dir.y = 0;
			player.dir.x = 1;
			player.rerender = TRUE;
		}
		else if(shooter_flash_timer == 1)
		{
			player.dir.x = 1;
			player.dir.y = 0;
			player.rerender = TRUE;
		}
		else
		{
			player.dir.x = 1;
			player.dir.y = 0;
		}
	}
	
	if(shooter_facing != shooter_old_facing)
	{
		player.rerender = TRUE;
	}
	shooter_old_facing = shooter_facing;

  // Actor Collisions
  hit_actor = ActorOverlapsPlayer(FALSE);
  if (hit_actor && hit_actor != NO_ACTOR_COLLISON && player_iframes == 0) {
    if (actors[hit_actor].collision_group) {
      player.hit_actor = 0;
      player.hit_actor = hit_actor;
    } else {
      player_iframes = SHOOTER_HURT_IFRAMES;
      ScriptStartBg(&actors[hit_actor].events_ptr, hit_actor);
    }
  }  
}
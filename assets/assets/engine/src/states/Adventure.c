#pragma bank 5

#include "states/Adventure.h"
#include "Actor.h"
#include "Camera.h"
#include "Collision.h"
#include "GameTime.h"
#include "Input.h"
#include "ScriptRunner.h"
#include "Scroll.h"
#include "Trigger.h"
#include "rand.h"

#define ADVENTURE_CAMERA_DEADZONE 8

// Collision bias, basically pushing the left/right boundaries inward. Play with this to match your player sprites best.
// Note that this many pixels is reduced from left AND right. Also you can probably break this with a number larger than 8.
UBYTE col_bias = 2;

void Start_Adventure() {
  // Set camera to follow player
  camera_offset.x = 0;
  camera_offset.y = 0;
  camera_deadzone.x = ADVENTURE_CAMERA_DEADZONE;
  camera_deadzone.y = ADVENTURE_CAMERA_DEADZONE;
}

void Update_Adventure() {
  WORD tile_x, tile_y;
  WORD tile_x1, tile_x2, tile_x3, tile_y1, tile_y2;
  UBYTE hit_actor = 0;
  UBYTE hit_trigger = 0;
  Vector2D backup_dir;

  player.moving = FALSE;

  // Move
  // player.dir.x = 0;
  if (INPUT_LEFT) {
    player.dir.x = -1;
    player.moving = TRUE;
    player.rerender = TRUE;
  } else if (INPUT_RIGHT) {
    player.dir.x = 1;
    player.moving = TRUE;
    player.rerender = TRUE;
  }

  // player.dir.y = 0;
  if (INPUT_UP) {
    player.dir.y = -1;
    player.moving = TRUE;
    player.rerender = TRUE;
  } else if (INPUT_DOWN) {
    player.dir.y = 1;
    player.moving = TRUE;
    player.rerender = TRUE;
  }

  if ((INPUT_LEFT || INPUT_RIGHT) && !INPUT_UP && !INPUT_DOWN) {
    player.dir.y = 0;
  } else if ((INPUT_UP || INPUT_DOWN) && !INPUT_LEFT && !INPUT_RIGHT) {
    player.dir.x = 0;
  }

  backup_dir.x = player.dir.x;
  backup_dir.y = player.dir.y;

  if (INPUT_A_PRESSED) {
    hit_actor = ActorInFrontOfPlayer(8, TRUE);
    if (hit_actor != NO_ACTOR_COLLISON) {
      ScriptStart(&actors[hit_actor].events_ptr);
    }
  }

  tile_x = (player.pos.x + 4) >> 3;
  tile_y = (player.pos.y) >> 3;

  // Check for trigger collisions
  if (ActivateTriggerAt(tile_x, tile_y, FALSE)) {
    // Landed on a trigger
    return;
  }


  // Collision checks
  // This for-loop setup is simply a way to be able to toggle which axis to resolve first. I thought it might be important.
  UBYTE i;
  UBYTE check_x_col_flag = TRUE; // arbitrarily picking X axis first to check
  for (i = 0; i < 2; i++) {
    if (check_x_col_flag) {
      // Left & Right Collision
      if (player.dir.x < 0) { // left
        tile_x1 = (player.pos.x - 1 + col_bias) >> 3;
        tile_x2 = (player.pos.x - 9 + col_bias) >> 3;
        tile_y1 = (player.pos.y) >> 3;
        tile_y2 = (player.pos.y + 7) >> 3;
        
        hit_actor = ActorAt1x2Tile(tile_x2, tile_y1, FALSE);
        if (hit_actor == NO_ACTOR_COLLISON) hit_actor = ActorAt1x2Tile(tile_x2, tile_y2, FALSE);
        if (TileAt(tile_x1, tile_y1) || TileAt(tile_x1, tile_y2) || (hit_actor && hit_actor != NO_ACTOR_COLLISON)) {
          player.dir.x = 0;
        }
      } else if (player.dir.x > 0) { // right
        tile_x1 = (player.pos.x + 16 - col_bias) >> 3;
        tile_y1 = (player.pos.y) >> 3;
        tile_y2 = (player.pos.y + 7) >> 3;
        
        hit_actor = ActorAt1x2Tile(tile_x1, tile_y1, FALSE);
        if (hit_actor == NO_ACTOR_COLLISON) hit_actor = ActorAt1x2Tile(tile_x1, tile_y2, FALSE);
        if (TileAt(tile_x1, tile_y1) || TileAt(tile_x1, tile_y2) || (hit_actor && hit_actor != NO_ACTOR_COLLISON)) {
          player.dir.x = 0;
        }
      }

      // Move player
      if (player.moving) {
        // Move actor
        if (player.move_speed == 0) {
          // Half speed only move every other frame
          if (IS_FRAME_2) {
            player.pos.x += (WORD)player.dir.x;
          }
        } else {
          player.pos.x += (WORD)(player.dir.x * player.move_speed);
        }
      }

      check_x_col_flag = FALSE;
    } else {
      // Up & Down Collision
      if (player.dir.y < 0) { // up
        tile_x1 = (player.pos.x + col_bias) >> 3;
        tile_x2 = (player.pos.x + 7) >> 3;
        tile_x3 = (player.pos.x + 15 - col_bias) >> 3;
        tile_y1 = (player.pos.y - 1) >> 3;

        hit_actor = ActorAt3x1Tile(tile_x1, tile_y1, FALSE);
        if (hit_actor == NO_ACTOR_COLLISON) hit_actor = ActorAt3x1Tile(((player.pos.x + 7 - col_bias) >> 3), tile_y1, FALSE);
        if (TileAt(tile_x1, tile_y1) || TileAt(tile_x2, tile_y1) || TileAt(tile_x3, tile_y1) || (hit_actor && hit_actor != NO_ACTOR_COLLISON)) {
          player.dir.y = 0;
        }
      } else if (player.dir.y > 0) { // down
        tile_x1 = (player.pos.x + col_bias) >> 3;
        tile_x2 = (player.pos.x + 7) >> 3;
        tile_x3 = (player.pos.x + 15 - col_bias) >> 3;
        tile_y1 = (player.pos.y + 8) >> 3;
        tile_y2 = (player.pos.y + 16) >> 3;

        hit_actor = ActorAt3x1Tile(tile_x1, tile_y2, FALSE);
        if (hit_actor == NO_ACTOR_COLLISON) hit_actor = ActorAt3x1Tile(((player.pos.x + 7 - col_bias) >> 3) , tile_y2, FALSE);
        if (TileAt(tile_x1, tile_y1) || TileAt(tile_x2, tile_y1) || TileAt(tile_x3, tile_y1) || (hit_actor && hit_actor != NO_ACTOR_COLLISON)) {
          player.dir.y = 0;
        }
      }

      // Move player
      if (player.moving) {
        // Move actor
        if (player.move_speed == 0) {
          // Half speed only move every other frame
          if (IS_FRAME_2) {
            player.pos.y += (WORD)player.dir.y;
          }
        } else {
          player.pos.y += (WORD)(player.dir.y * player.move_speed);
        }
      }

      check_x_col_flag = TRUE;
    }
  }

  if (hit_actor) {
    if (player_iframes == 0) {
      if (actors[hit_actor].collision_group) {
        player.hit_actor = 0;
        player.hit_actor = hit_actor;
      } else {
        player_iframes = 10;
      }
    }
  }
    
  // keep direction when inputting against a wall in one direction
  if (player.moving) {
    if (!(player.dir.x > 0 | player.dir.x<0 | player.dir.y> 0 | player.dir.y < 0)) {
      player.moving = FALSE;
      player.dir.x = backup_dir.x;
      player.dir.y = backup_dir.y;
    }
  }

}

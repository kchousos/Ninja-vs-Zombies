#include "ADTList.h"
#include "raylib.h"

#include "state.h"
#include "interface.h"

// Assets
Texture character_running_forward;
Texture character_running_backwards;
Texture character_jumping_forward;
Texture character_jumping_backwards;
Texture enemy_forward;
Texture enemy_backwards;
Texture background;
Texture portal;
Texture obstacle;
Music soundtrack;
Sound portal_sound;
Sound win;

void draw_logo();

void interface_init() {
	// Αρχικοποίηση του παραθύρου
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Awesome Game");
	SetTargetFPS(60);
  InitAudioDevice();
	
	// Φόρτωση εικόνων και ήχων
	character_running_forward = LoadTextureFromImage(LoadImage("assets/character_running_forward.png"));
	character_running_backwards = LoadTextureFromImage(LoadImage("assets/character_running_backwards.png"));
	character_jumping_forward = LoadTextureFromImage(LoadImage("assets/character_jumping_forward.png"));
	character_jumping_backwards = LoadTextureFromImage(LoadImage("assets/character_jumping_backwards.png"));
	enemy_forward = LoadTextureFromImage(LoadImage("assets/enemy_forward.png"));
	enemy_backwards = LoadTextureFromImage(LoadImage("assets/enemy_backwards.png"));
	soundtrack = LoadMusicStream("assets/soundtrack.mp3");
	background = LoadTextureFromImage(LoadImage("assets/background.png"));
	portal = LoadTextureFromImage(LoadImage("assets/portal.png"));
	portal_sound = LoadSound("assets/portal_sound.mp3");
	win = LoadSound("assets/win.mp3");
	obstacle = LoadTextureFromImage(LoadImage("assets/obstacle.png"));

	// Το αρχικό raylib splash screen
	draw_logo();

	// Αύξηση έντασης των sfx ήχων
	SetSoundVolume(portal_sound, 2.3);
	SetSoundVolume(win, 3.0);

	// Εκκίνηση της μουσικής
	PlayMusicStream(soundtrack);
}

void interface_close() {
	UnloadTexture(character_running_forward);
	UnloadTexture(character_running_backwards);
	UnloadTexture(character_jumping_forward);
	UnloadTexture(character_jumping_backwards);
	UnloadTexture(enemy_forward);
	UnloadTexture(enemy_backwards);
	UnloadTexture(obstacle);
	UnloadTexture(portal);
	UnloadTexture(background);
	UnloadMusicStream(soundtrack);
	UnloadSound(portal_sound);
	UnloadSound(win);
	CloseAudioDevice();
	CloseWindow();
}

// Draw game (one frame)
void interface_draw_frame(State state) {
	BeginDrawing();

	int floor = SCREEN_HEIGHT * 0.87;
	int char_pos = SCREEN_WIDTH * 0.25;
	StateInfo info = state_info(state);

	// Ο ήχος που παίζει όταν το wins αυξάνεται κατά ένα.
	// Ακούγεται όταν ο χαρακτήρας είναι στο χ == 0 και
	// βρίσκεται σε jumping state
	if (info->character->rect.x == 0 && info->character->jumping)
		PlaySound(win);

	// Έλεγχος για enter όταν έχεις χάσει
	if (!info->playing && IsKeyDown(KEY_ENTER)) {

		info->character->rect.x = 0;
		info->playing = true;
	}

	// Το tint των εικόνων, ανάλογα με το αν
	// Το game είναι paused ή όχι.
	Color tint;
	if (info->paused)
		tint = GRAY;
	else
		tint = WHITE;

	if (!IsMusicPlaying(soundtrack))
		ResumeMusicStream(soundtrack);
	else if (!info->paused)
		UpdateMusicStream(soundtrack);

	// Καθαρισμός, θα τα σχεδιάσουμε όλα από την αρχή
	ClearBackground(RAYWHITE);
	DrawTexture(background, 0, 0, tint);

	if (info->paused)
		DrawText("PAUSED",
			 GetScreenWidth() / 2 - MeasureText("PAUSED", 50) / 2,
			 GetScreenHeight() / 2 - 50, 50, WHITE);
	
	// Το -200 και +200 είναι ώστε τα γραφικά να είναι πιο ομαλά,
	// καθώς το ότι το x μιας εικόνας/αντικειμένου δεν είναι στην
	// οθόνη, δε σημαίνει ότι δεν θα έπρεπε να φαίνεται κανένα
	// σημείο της εικόνας
	List current_objects = state_objects(state,
				info->character->rect.x - char_pos - 200,
		    info->character->rect.x + SCREEN_WIDTH + 200);

	for (ListNode node = list_first(current_objects);
			 node != LIST_EOF;
			 node = list_next(current_objects, node)) {

		Object obj = list_node_value(current_objects, node);

		if (obj->type == ENEMY) {
			if (obj->forward) {
				DrawTexture(enemy_backwards, char_pos + (obj->rect.x - info->character->rect.x) - 50,
										obj->rect.y + floor, tint);
			} else {
				DrawTexture(enemy_forward,char_pos + (obj->rect.x - info->character->rect.x) - 50,
										obj->rect.y + floor, tint);
			}
		}
		
		if (obj->type == OBSTACLE) {
				DrawTexture(obstacle, char_pos + (obj->rect.x - info->character->rect.x),
										obj->rect.y + floor/1.3 + obj->rect.height, tint);
		}
		
		if (obj->type == PORTAL) {
				DrawTexture(portal,char_pos + (obj->rect.x - info->character->rect.x),
										obj->rect.height + floor/1.45, tint);

				if (CheckCollisionRecs(info->character->rect, obj->rect))
					PlaySound(portal_sound);
		}
	}

	if (floor + info->character->rect.y != floor - info->character->rect.height) {
		if (info->character->forward) {
			DrawTexture(character_jumping_forward, char_pos,
								floor + info->character->rect.y, tint);
			} else {
		DrawTexture(character_jumping_backwards, char_pos,
								floor + info->character->rect.y, tint);
		}
		} else {
		if (info->character->forward) {
			DrawTexture(character_running_forward,char_pos, floor + info->character->rect.y, tint);
		}
		else {
			DrawTexture(character_running_backwards, char_pos, floor + info->character->rect.y, tint);
		}
	}

	// Σχεδιάζουμε το σκορ και το FPS counter
	DrawText("PORTAL:", 20, 20, 40, tint);
	DrawText(TextFormat("%03i", info->current_portal), 210, 20, 40, tint);
	DrawText(TextFormat("/%03i", PORTAL_NUM), 280, 20, 40, tint);
	DrawText("WINS:", 20, 60, 40, tint);
	DrawText(TextFormat("%03i", info->wins), 140, 60, 40, tint);
	DrawFPS(SCREEN_WIDTH - 80, 0);

	// Αν το παιχνίδι έχει τελειώσει, σχεδιάζουμε το μήνυμα για να ξαναρχίσει
	if (!info->playing) {
		PauseMusicStream(soundtrack);
		DrawText(
			"PRESS [ENTER] TO PLAY AGAIN",
			 GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 50) / 2,
			 GetScreenHeight() / 2 - 50, 50, WHITE);
		info->character->rect.y = - info->character->rect.height;
		info->character->jumping = false;
	}

	EndDrawing();
}

void draw_logo() {

  int logoPositionX = SCREEN_WIDTH/2 - 128;
  int logoPositionY = SCREEN_HEIGHT/2 - 128;

  int topSideRecWidth = 16;
  int leftSideRecHeight = 16;
	int framesCounter = 0;
  int lettersCount = 0;

  int bottomSideRecWidth = 16;
  int rightSideRecHeight = 16;
	float alpha = 1.0f;       
	int logo_state = 0;

	while (logo_state < 4 &&
				 !IsKeyDown(KEY_SPACE) &&
				 !IsKeyDown(KEY_ENTER)) {
		if (logo_state == 0) {
			framesCounter++;
			
			if (framesCounter == 120) {
				logo_state = 1;
				framesCounter = 0;
			}
		}
		else if (logo_state == 1) {
			topSideRecWidth += 4;
			leftSideRecHeight += 4;
			
			if (topSideRecWidth == 256) logo_state = 2;
		}
		else if (logo_state == 2) {
			bottomSideRecWidth += 4;
			rightSideRecHeight += 4;
			
			if (bottomSideRecWidth == 256) logo_state = 3;
		}
		else if (logo_state == 3) {
			framesCounter++;
			
			if (framesCounter/12) {
				lettersCount++;
				framesCounter = 0;
			}
			
			if (lettersCount >= 10) {
				alpha -= 0.02f;
				
				if (alpha <= 0.0f) {
					alpha = 0.0f;
					logo_state = 4;
				}
			}
		}
		else if (logo_state == 4) {
			if (IsKeyPressed(KEY_R)) {
				framesCounter = 0;
				lettersCount = 0;
				
				topSideRecWidth = 16;
				leftSideRecHeight = 16;
				
				bottomSideRecWidth = 16;
				rightSideRecHeight = 16;
				
				alpha = 1.0f;
				logo_state = 0;     
			}
		}
		BeginDrawing();
		
		ClearBackground(RAYWHITE);
		
		if (logo_state == 0) {
			if ((framesCounter/15)%2) DrawRectangle(logoPositionX, logoPositionY, 16, 16, BLACK);}
		else if (logo_state == 1) {
			DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
			DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);
		}
		else if (logo_state == 2) {
			DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
			DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);
			
			DrawRectangle(logoPositionX + 240, logoPositionY, 16, rightSideRecHeight, BLACK);
			DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, BLACK);
		}
		else if (logo_state == 3) {
			DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(BLACK, alpha));
			DrawRectangle(logoPositionX, logoPositionY + 16, 16,
										leftSideRecHeight - 32, Fade(BLACK, alpha));
			
			DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16,
										rightSideRecHeight - 32, Fade(BLACK, alpha));
			DrawRectangle(logoPositionX, logoPositionY + 240,
										bottomSideRecWidth, 16, Fade(BLACK, alpha));
			
			DrawRectangle(SCREEN_WIDTH/2 - 112, SCREEN_HEIGHT/2 - 112, 224, 224, Fade(RAYWHITE, alpha));
			
			DrawText(TextSubtext("raylib", 0, lettersCount), SCREEN_WIDTH/2 - 44,
							 SCREEN_HEIGHT/2 + 48, 50, Fade(BLACK, alpha));
		}

		EndDrawing();
	}
}

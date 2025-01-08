#include "raylib.h"
#include <cmath>

/*
 *===========================
 * World Data
 *===========================
 */
#define mapWidth 8
#define mapHeight 8

const int worldMap[ mapWidth ][ mapHeight ] = {
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 1, 0, 1, 1, 0, 1 },
	{ 1, 0, 1, 0, 0, 0, 0, 1 },
	{ 1, 0, 0, 0, 0, 1, 0, 1 },
	{ 1, 0, 1, 1, 0, 1, 0, 1 },
	{ 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 }
};

// Function prototypes (to make modular)
void HandlePlayerMovement( const Vector2 &position, const Vector2 &direction,
                           const Vector2 &cameraPlane, float       deltaTime );

void PerformRayCasting( Vector2 playerPosition, Vector2 playerDirection,
                        Vector2 cameraPlane, int        screenWidth, int screenHeight );

int main() {
	// Screen Initialization
	constexpr int screenWidth  = 800;
	constexpr int screenHeight = 600;

	InitWindow( screenWidth, screenHeight, "~Fortune's Tale~" );

	/*=================
	 * Player Variables
	 *=================
	 */
	Vector2 playerPosition  = { 3.5f, 3.5f };  // Player starts in the middle
	Vector2 playerDirection = { -1.0f, 0.0f }; // Player faces negative X-Axis, initially.
	Vector2 cameraPlane     = { 0.0f, 0.66f }; // Camera FOV plane.

	SetTargetFPS( 60 );

	// Main Loop
	while( !WindowShouldClose() ) {
		const float deltaTime = GetFrameTime(); // Frame time for consistent movement.

		// Handle Movement
		HandlePlayerMovement( playerPosition, playerDirection, cameraPlane, deltaTime );

		// Decorating Screen
		BeginDrawing();
		ClearBackground( BLACK );

		// Render
		PerformRayCasting( playerPosition, playerDirection, cameraPlane, screenWidth, screenHeight );

		// Debug Info
		DrawText( TextFormat( "POS: (%.2f, %.2f)", playerPosition.x, playerPosition.y ),
		          10, 10, 20, RAYWHITE );
		DrawText( TextFormat( "DIR: (%.2f, %.2f)", playerDirection.x, playerDirection.y ),
		          10, 40, 20, RAYWHITE );
		DrawText( "Use WASD to move and rotate", 10, 70, 20, GRAY );

		EndDrawing();
	}

	// Tidy up and close window.
	CloseWindow();
	return 0;
}

/*
 * =====================
 * Handle WASD Movement
 * =====================
 */
void HandlePlayerMovement( const Vector2 &position, const Vector2 &direction, const Vector2 &cameraPlane,
                           const float    deltaTime ) {
	const float moveSpeed = 3.0f * deltaTime;
	const float rotSpeed  = 1.5f * deltaTime;

	if( IsKeyDown( KEY_W ) ) {
		if( worldMap[ ( int ) ( position.x + direction.x * moveSpeed ) ][ ( int ) position.y ] == 0 )
			position.x += direction.x * moveSpeed;
		if( worldMap[ ( int ) position.x ][ ( int ) ( position.y + direction.y * moveSpeed ) ] == 0 )
			position.y += direction.y * moveSpeed;
	}
	if( IsKeyDown( KEY_S ) ) {
		if( worldMap[ ( int ) ( position.x - direction.x * moveSpeed ) ][ ( int ) position.y ] == 0 )
			position.x -= direction.x * moveSpeed;
		if( worldMap[ ( int ) position.x ][ ( int ) ( position.y - direction.y * moveSpeed ) ] == 0 )
			position.y -= direction.y * moveSpeed;
	}
	if( IsKeyDown( KEY_A ) ) {
		const float oldDirX = direction.x;
		direction.x         = direction.x * cos( rotSpeed ) - direction.y * sin( rotSpeed );
		direction.y         = oldDirX * sin( rotSpeed ) + direction.y * cos( rotSpeed );

		const float oldPlaneX = cameraPlane.x;
		cameraPlane.x         = cameraPlane.x * cos( rotSpeed ) - cameraPlane.y * sin( rotSpeed );
		cameraPlane.y         = oldPlaneX * sin( rotSpeed ) + cameraPlane.y * cos( rotSpeed );
	}
	if( IsKeyDown( KEY_D ) ) {
		const float oldDirX = direction.x;
		direction.x         = direction.x * cos( -rotSpeed ) - direction.y * sin( -rotSpeed );
		direction.y         = oldDirX * sin( -rotSpeed ) + direction.y * cos( -rotSpeed );

		const float oldPlaneX = cameraPlane.x;
		cameraPlane.x         = cameraPlane.x * cos( -rotSpeed ) - cameraPlane.y * sin( -rotSpeed );
		cameraPlane.y         = oldPlaneX * sin( -rotSpeed ) + cameraPlane.y * cos( -rotSpeed );
	}
}

float operator*( float lhs, const Vector2 &rhs );

/*
 * ======================
 *	Perform RayCasting
 * ======================
 */
void PerformRayCasting( Vector2 playerPosition, Vector2 playerDirection, Vector2 cameraPlane, int screenWidth,
                        int     screenHeight ) {
	for( int x = 0; x < screenWidth; x++ ) {
		// Calculate ray direction for this column
		const float cameraX = 2 * x / ( float ) screenWidth - 1; // X in camera space

		Vector2 rayDirection = {
			playerDirection.x + cameraPlane.x * cameraX,
			playerDirection.y + cameraPlane.y * cameraX
		};

		// Map Position
		int mapX = ( int ) playerPosition.x;
		int mapY = ( int ) playerPosition.y;

		// Length of ray from current position to next side
		Vector2 sideDist;

		// Length of ray from one side to the next
		const Vector2 deltaDist = {
			fabs( 1 / rayDirection.x ),
			fabs( 1 / rayDirection.y )
		};
		float perpWallDist;

		// Step direction and side distances
		Vector2 step;

		if( rayDirection.x < 0 ) {
			step.x     = -1;
			sideDist.x = ( playerPosition.x - mapX ) * deltaDist.x;
		} else {
			step.x     = 1;
			sideDist.x = ( mapX + 1.0 - playerPosition.x ) * deltaDist.x;
		}

		if( rayDirection.y < 0 ) {
			step.y     = -1;
			sideDist.y = ( playerPosition.y - mapY ) * deltaDist.y;
		} else {
			step.y     = 1;
			sideDist.y = ( mapY + 1.0 - playerPosition.y ) * deltaDist.y;
		}

		// Perform DDA (Digital Differential Analysis)
		int hit  = 0;
		int side = 0; // Wall side (vertical or horizontal)
		while( hit == 0 ) {
			// Jump to next map square in ray direction
			if( sideDist.x < sideDist.y ) {
				sideDist.x += deltaDist.x;
				mapX += step.x;
				side = 0;
			} else {
				sideDist.y += deltaDist.y;
				mapY += step.y;
				side = 1;
			}
			if( worldMap[ mapX ][ mapY ] > 0 ) hit = 1; // Wall hit!
		}

		// Calculate perpendicular wall distance
		if( side == 0 ) perpWallDist = ( mapX - playerPosition.x + ( 1 - step.x ) / 2 ) / rayDirection.x;
		else perpWallDist            = ( mapY - playerPosition.y + ( 1 - step.y ) / 2 ) / rayDirection.y;

		// Calculate height of the wall slice
		const int lineHeight = ( int ) ( screenHeight / perpWallDist );

		// Determine start and end for this column
		int drawStart = -lineHeight / 2 + screenHeight / 2;
		if( drawStart < 0 ) drawStart = 0;

		int drawEnd = lineHeight / 2 + screenHeight / 2;
		if( drawEnd >= screenHeight ) drawEnd = screenHeight - 1;

		// Assign wall color based on side
		const Color wallColor = ( side == 0 ) ? BLUE : DARKBLUE;

		// Draw Slice
		DrawLine( x, drawStart, x, drawEnd, wallColor );
	}
}


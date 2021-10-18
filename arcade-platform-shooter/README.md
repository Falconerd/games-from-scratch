# Arcade platformer shooter

Music from:
    https://ryanavx.itch.io/breezys-mega-quest

## TODO in priority order
    - [/] Circle shader
        - Draw circle on a quad, move the quad
        - [?] Also need to scale in the pixel shader to keep pixel perfect
    - [X] Rocket launcher circle animation
        - When the rocket hits something, a shockwave should expand from the impact point
    - [X] Screen-shake
        - Just use stacking timer and magnitude, offset projection?
    - [X] Rocket launcher damage 
        - Find all entities within radius, probably just query all entities since the game is small
    - [X] Weapon knockback/kick
    - [X] Pistol weapon
    - [X] Revolver weapon
        - [X] Large bullets
    - [X] Bullets should not collide more than once with the same enemy
    - [X] Enemy death animations
        - [X] Perhaps make enemies fall off screen
        - [X] Rotation
    - [X] Rocket launcher acceleration
    - [X] Rocket launcher trail smoke
    - [X] Text rendering using FreeType
    - [X] Simple input system
    - [X] Audio
    - [X] Draw animations
        - [X] Player
        - [X] Small enemy
        - [X] Large enemy
    - [X] Sprite animation system
        - Probably use a simple FSM... Maybe link to entity system
        - [X] Or just use an explicit "entity.animation = x"
    - [/] Draw the current weapon at the player position
    - [ ] Resolution stuff
        - Currently just runs in windowed mode in 1080p 
    - [X] Profiler
        - [X] Would be nice to see FPS, frame time, etc?
        - nsight, radeon profiler

    1. [/] Entities are currently misaligned with their hit boxes.
    2. [ ] Weapons are not showing up with the correct offset (they have been
       moved to a separate sprite from the player).
    3. [ ] The player has no idle animation.
    4. [ ] The enemies are currently not displaying as their "angry" version,
       though it seems their speed is increased when respawning.
    5. [ ] Score text is off-centre.
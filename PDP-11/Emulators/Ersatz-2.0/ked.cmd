!+
!
! Key scripts for cursor keys using KED or EDT under E11.
!
! Cursor keys are made to appear directionless by using FLAG1 to keep track
! of the current direction.
!
! By John Wilson.
!
! 06/18/97	JMBW	Created.
!
!-
! Keypad 4 key has usual definition but clears FLAG1
!
DEFINE KEYPRESS KP4 = &
 IF NOT CTRL THEN               ! Ctrl suppresses all keypad keys &
  IF EKB OR NUM THEN            ! keypad is not acting as cursor keys &
   IF APPKEYPAD THEN            ! application keypad mode (used by KED) &
    CLEAR FLAG1 :               ! going forwards &
    IF VT52 THEN &
     CHR$(27)+"?t" &
    ELSE &
     CHR$(27)+"Ot" &
    ENDIF &
   ELSE &
    "4"                         ! numeric keypad mode &
   ENDIF &
  ELSE &
   PRESS LARROW                 ! keypad works as cursor keys (84-key KB) &
  ENDIF &
 ENDIF
!
! Keypad 5 key has usual definition but sets FLAG1
!
DEFINE KEYPRESS KP5 = &
 IF NOT CTRL THEN               ! Ctrl suppresses all keypad keys &
  IF EKB OR NUM THEN            ! keypad is not acting as cursor keys &
   IF APPKEYPAD THEN            ! application keypad mode (used by KED) &
    SET FLAG1 :                 ! going backwards &
    IF VT52 THEN &
     CHR$(27)+"?u" &
    ELSE &
     CHR$(27)+"Ou" &
    ENDIF &
   ELSE &
    "5"                         ! numeric keypad mode &
   ENDIF &
  ENDIF                         ! no op in cursor key mode (84-key KB) &
 ENDIF
!
! Other cursor keys now depend on FLAG1
!
DEFINE KEYPRESS INS =           ! insert blank line &
 PRESS NUMLOCK : &
 PRESS KP0
DEFINE KEYPRESS DEL = &
 IF FLAG1 THEN                  ! KED is going backwards &
  PRESS KP4 :                   ! switch forwards temporarily &
  IF VT52 THEN                  ! send code for keypad "," key &
   CHR$(27)+"?l" &
  ELSE &
   CHR$(27)+"Ol" &
  ENDIF : &
  PRESS KP5                     ! switch backwards again &
 ELSE &
  IF VT52 THEN                  ! send code for keypad "," key &
   CHR$(27)+"?l" &
  ELSE &
   CHR$(27)+"Ol" &
  ENDIF &
 ENDIF
DEFINE KEYPRESS HOME =          ! backspace doesn't do it in KED &
 IF FLAG1 THEN                  ! KED is going backwards &
  PRESS KP0                     ! easy, "0" key does it &
 ELSE &
  PRESS KP5 :                   ! switch backwards temporarily &
  PRESS KP0 :                   ! go to begn of line &
  PRESS KP4                     ! switch forwards again &
 ENDIF
DEFINE KEYPRESS END = &
 IF FLAG1 THEN                  ! KED is going backwards &
  PRESS KP4 :                   ! switch forwards temporarily &
  PRESS KP2 :                   ! jump to end of line &
  PRESS KP5                     ! switch backwards again &
 ELSE &
  PRESS KP2                     ! jump to end of line &
 ENDIF
DEFINE KEYPRESS PGUP = &
 IF FLAG1 THEN                  ! KED is going backwards &
  PRESS KP8                     ! up a section &
 ELSE &
  PRESS KP5 :                   ! switch backards temporarily &
  PRESS KP8 :                   ! up a section &
  PRESS KP4                     ! switch forewards again &
 ENDIF
DEFINE KEYPRESS PGDN = &
 IF FLAG1 THEN                  ! KED is going backwards &
  PRESS KP4 :                   ! switch forwards temporarily &
  PRESS KP8 :                   ! down a section &
  PRESS KP5                     ! switch backwards again &
 ELSE &
  PRESS KP8                     ! down a section &
 ENDIF

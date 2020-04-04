#include <Arduboy.h>

#define CHECK 0x80
#define MARK 0x40
#define MINE 0x20
#define COUNT 0x1F
#define ON_GOING 0
#define WIN 1
#define LOSS 2
Arduboy arduboy;
char board[64] = {0};
byte cur_r, cur_c;
byte old_r, old_c;
byte old_state, cur_state;
byte game_state;
byte rem;

void set_mines(void) {
  memset(board, 0, 64);
  rem = 54;
  int res;
  for (int i = 0; i < 10; ++i) {
    for (res = rand() % 64; board[res] & MINE; res = rand() % 64);
    board[res] |= MINE; 
  }
  char *ptr = board;
  char *temp;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 16; ++j, ++ptr) {
      if (i) {
        for (temp = j ? ptr - 17 : ptr - 16; temp < (j < 15 ? ptr - 14 : ptr - 15); *ptr += (*(temp++) & MINE) != 0);
      } 
      for (temp = j ? ptr - 1 : ptr; temp < (j < 15 ? ptr + 2 : ptr + 1); *ptr += (*(temp++) & MINE) != 0);
      if (i < 3) {
        for (temp = j ? ptr + 15 : ptr + 16; temp < (j < 15 ? ptr + 18 : ptr + 17); *ptr += (*(temp++) & MINE) != 0);
      }
    }
  }
}

void mark(int r, int c) {
  int pos = r * 16 + c;
  if (board[pos] & CHECK) {
    return;
  }
  board[pos] ^= MARK;
}

void check(int r, int c) {
  static int stack[64];
  int *stack_ptr = stack;
  int pos = r * 16 + c;
  if (board[pos] & MINE) {
    game_state = LOSS;
    return;
  }
  if (board[pos] & MARK) {
    return;
  }
  if (!(--rem)) {
    game_state = WIN;
    return;
  }
  board[pos] |= CHECK;
  if (!(board[pos] & COUNT)) {
    *(stack_ptr++) = r * 16 + c;
  }
  for (; stack_ptr > stack;) {
    int ind = *(--stack_ptr);
    int rr, cc;
    rr = ind / 16;
    cc = ind % 16;
    for (int i = rr ? rr - 1 : 0; i < (rr < 3 ? rr + 2 : 4); ++i) {
      for (int j = cc ? cc - 1 : 0; j < (cc < 15 ? cc + 2 : 16); ++j) {
        int indd = i * 16 + j;
        if ((board[indd] & CHECK) || (board[indd] & MINE)) continue;
        board[indd] |= CHECK;
        if (!(board[indd] & COUNT)) {
          *(stack_ptr++) = indd;
        }
        if (!(--rem)) {
          game_state = WIN;
          return;
        }
      }
    }
  }
}

void render(bool reveal) {
  char *ptr = board;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 16; ++j, ++ptr) {
      if (i == cur_r && j == cur_c) continue;
      arduboy.setCursor(16 + j * 7, 14 + i * 9);
      if (reveal) {
        if (*ptr & MINE) {
          arduboy.print('*');
        } else {
          arduboy.print(*ptr & COUNT);
        }
      } else {
        if (*ptr & CHECK) {
          arduboy.print(*ptr & COUNT);
        }
        if (*ptr & MARK) {
          arduboy.print('F');
        }
      }
    }
  }
}

void clear() {
  char *ptr = board;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 16; ++j, ++ptr) {
      if (i == cur_r && j == cur_c) continue;
      arduboy.fillRect(16 + j * 7, 14 + i * 9, 5, 8, 0);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  arduboy.begin();  
  arduboy.setFrameRate(60);
  arduboy.initRandomSeed();
  arduboy.clear();
  set_mines();
  for (int i = 0; i < 17; ++i) {
    arduboy.drawFastVLine(15 + i * 7, 13, 36, 1);
    if (i < 5) {
      arduboy.drawFastHLine(15, 13 + i * 9, 112, 1);
    }
  }
  arduboy.fillRect(16, 14, 5, 8, 1);
  cur_r = cur_c = old_r = old_c = 0;
  old_state = cur_state = 0;
  game_state = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!arduboy.nextFrame()) {
    return;  
  }
  cur_state = arduboy.buttonsState();
  switch (game_state) {
  case ON_GOING:
    
    if (cur_c != old_c || cur_r != old_r) {
      arduboy.fillRect(16 + old_c * 7, 14 + old_r * 9, 5, 8, 0);
      if (board[old_r * 16 + old_c] & CHECK) {
        arduboy.setCursor(16 + old_c * 7, 14 + old_r * 9);
        arduboy.print(board[old_r * 16 + old_c] & COUNT);
      }
      if (board[old_r * 16 + old_c] & MARK) {
        arduboy.setCursor(16 + old_c * 7, 14 + old_r * 9);
        arduboy.print('F');
      }
      arduboy.fillRect(16 + cur_c * 7, 14 + cur_r * 9, 5, 8, 1);
      old_c = cur_c;
      old_r = cur_r;
    }
    if ((cur_state & LEFT_BUTTON) && !(old_state & LEFT_BUTTON)) {
      --cur_c;
      cur_c &= 0xf;
    }
    if ((cur_state & RIGHT_BUTTON) && !(old_state & RIGHT_BUTTON)) {
      ++cur_c;
      cur_c &= 0xf;
    }
    if ((cur_state & UP_BUTTON) && !(old_state & UP_BUTTON)) {
      --cur_r;
      cur_r &= 0x3;
    }
    if ((cur_state & DOWN_BUTTON) && !(old_state & DOWN_BUTTON)) {
      ++cur_r;
      cur_r &= 0x3;
    }
    if ((cur_state & A_BUTTON) && !(old_state & A_BUTTON)) {
      check(cur_r, cur_c);
      render(false);
    }
    if ((cur_state & B_BUTTON) && !(old_state & B_BUTTON)) {
      mark(cur_r, cur_c);
      render(false);
    }
    switch (game_state) {
    case WIN:
      arduboy.setCursor(0, 0);
      arduboy.print("Win");
      render(true);
      break;
    case LOSS:
      arduboy.setCursor(0, 0);
      arduboy.print("Loss");
      render(true);
      break;
    }
    break;
  case WIN:
  case LOSS:
    if (cur_state && !old_state) {
      set_mines();
      arduboy.setCursor(0, 0);
      arduboy.print("    ");
      clear();
      game_state = ON_GOING;
    }
    break;
  }
  old_state = cur_state;
  arduboy.display();
}

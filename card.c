#include <stdlib.h>

#include "card.h"

struct card {
  Card *prev;
  Card *next;
  char up;
  char suit;
  char rank;
};

Card *new_card(char suit, char rank) {
  Card *card = malloc(sizeof(Card));
  card->prev = NULL;
  card->next = NULL;
  card->up = 1;
  card->suit = suit;
  card->rank = rank;
  return card;
}

Card *new_deck() {
  Card *deck = new_card(BOTTOM, 0);
  Card *prev = deck;
  for (char suit = 0; suit < 4; suit++) {
    for (char rank = 1; rank <= 13; rank++) {
      Card *card = new_card(SUITS[suit], rank);
      prev->next = card;
      card->prev = prev;
      prev = card;
    }
  }
  return deck;
}

Card *shuffle_stack(Card *stack) {
  int n = 2;
  if (!stack->next) {
    return stack;
  }
  Card *new = stack;
  Card *next = stack->next;
  new->next = NULL;
  new->prev = NULL;
  while (next) {
    Card *next_next = next->next;
    int i = rand() % n;
    if (i == 0) {
      new->prev = next;
      next->prev = NULL;
      next->next = new;
      new = next;
    } else {
      Card *c = new;
      for (int j = 1; j < i; j++) {
        c = c->next;
      }
      if (c->next) {
        c->next->prev = next;
      }
      next->next = c->next;
      c->next = next;
      next->prev = c;
    }
    n++;
    next = next_next;
  }
  return new;
}

Card *take_card(Card *card) {
  if (card->prev) {
    card->prev->next = card->next;
  }
  if (card->next) {
    card->next->prev = card->prev;
  }
  card->prev = NULL;
  card->next = NULL;
  return card;
}

Card *take_stack(Card *stack) {
  if (stack->prev) {
    stack->prev->next = NULL;
  }
  stack->prev = NULL;
  return stack;
}

void move_stack(Card *dest, Card *src) {
  if (dest->next)
    return move_stack(dest->next, src);
  dest->next = src;
  if (src->prev) src->prev->next = NULL;
  src->prev = dest;
}

Card *get_bottom(Card *stack) {
  if (stack->prev)
    return get_bottom(stack->prev);
  return stack;
}

char get_stack_type(Card *stack) {
  return get_bottom(stack)->suit;
}


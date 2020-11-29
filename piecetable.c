#include <stdlib.h>
#include <string.h>

#include "list.h"

#include "piecetable.h"

#define ORIGINAL 0
#define ADD 1

struct piece {
    // TODO: Performance could probably be improved by referencing the specific char *
    char which; // 0 = "original" // 1 = "add"
    int start;
    int length;
};
typedef struct piece * Piece;

struct piecetable {
    char * original; 
    List add;    // List<char *>
    List pieces; // List<Piece>
    int length;  // Character count of the current value
};

Piecetable piecetable_create(char * original) {
    Piecetable pt = malloc(sizeof(struct piecetable));
    pt->original = strdup(original);
    pt->add = list_create();
    pt->pieces = list_create();

    pt->length = strlen(original);

    Piece initialPiece = malloc(sizeof(struct piece));
    initialPiece->which = ORIGINAL;
    initialPiece->start = 0;
    initialPiece->length = strlen(original);
    list_append(pt->pieces, initialPiece);

    return pt;
}

void piecetable_free(Piecetable pt) {
    list_free(pt->pieces);
    list_free(pt->add);
    free(pt->original);
    free(pt);
}

int piecetable_add_length(Piecetable pt) {
    int length = 0;
    ListItem curr_item = list_get_first(pt->add);
    while(curr_item) {
        length += strlen(curr_item->value);
        curr_item = curr_item->next;
    }
    return length;
}

void piecetable_insert(Piecetable pt, char * value, int at) {
    int add_length_before_insert = piecetable_add_length(pt);

    list_append(pt->add, strdup(value));

    Piece newPiece = malloc(sizeof(struct piece));
    newPiece->which = ADD;
    newPiece->start = add_length_before_insert;
    newPiece->length = strlen(value);

    if(at == 0) {
        list_insert(pt->pieces, 0, newPiece);
    } else {
        ListItem piece_item = list_get_first(pt->pieces);
        Piece piece;
        int offset = 0;
        while(piece_item) {
            piece = piece_item->value;

            if(piece->length > at - 1 - offset) {
                /* Insert must happen inside or after the current piece */
                if(at - offset == piece->length) {
                    /* Insert after this piece */
                    ListItem newPieceItem = malloc(sizeof(struct list_item));
                    newPieceItem->value = newPiece;
                    newPieceItem->next = piece_item->next;
                    piece_item->next = newPieceItem;
                } else {
                    /* Insert inside this piece */
                    int oldLength = piece->length;
                    piece->length = at - offset;

                    Piece afterPiece = malloc(sizeof(struct piece));
                    afterPiece->which = piece->which;
                    afterPiece->start = piece->start + piece->length;
                    afterPiece->length = oldLength - piece->length;

                    ListItem newPieceItem = malloc(sizeof(struct list_item));
                    newPieceItem->value = newPiece;
                    newPieceItem->next = piece_item->next;
                    piece_item->next = newPieceItem;

                    ListItem afterPieceItem = malloc(sizeof(struct list_item));
                    afterPieceItem->value = afterPiece;
                    afterPieceItem->next = newPieceItem->next;
                    newPieceItem->next = afterPieceItem;
                }

                break;
            } else {
                offset += piece->length;
            }

            piece_item = piece_item->next;
        }
    }
    pt->length += strlen(value);

}

char * piecetable_value(Piecetable pt) {
    ListItem curr_piece_item = list_get_first(pt->pieces);
    
    char * value = malloc(pt->length + 1);
    
    Piece curr_piece;
    while(curr_piece_item) {
        curr_piece = curr_piece_item->value;

        if(curr_piece->which == ORIGINAL) {
            strncat(
                value,
                pt->original + curr_piece->start,
                curr_piece->length
            );
        } else {
            ListItem curr_add_item = list_get_first(pt->add);

            int offset = 0;
            char * curr_add;
            int curr_add_length;
            
            while(curr_add_item) {
                curr_add = curr_add_item->value;
                curr_add_length = strlen(curr_add);

                if(curr_piece->start < offset + curr_add_length) {
                    strncat(
                        value,
                        curr_add + (curr_piece->start - offset),
                        curr_piece->length
                    );
                    break;
                } else {
                    offset += curr_add_length;
                    curr_add_item = curr_add_item->next;
                }
            }
        }
        
        curr_piece_item = curr_piece_item->next;
    }

    return value;
}

#undef ORIGINAL
#undef ADD

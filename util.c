#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <arpa/inet.h>

void
hexdump (FILE *fp, void *data, size_t size) {
    int offset, index;
    unsigned char *src;

    src = (unsigned char *)data;
    fprintf(fp, "+------+-------------------------------------------------+------------------+\n");
    for(offset = 0; offset < (int)size; offset += 16) {
        fprintf(fp, "| %04x | ", offset);
        for(index = 0; index < 16; index++) {
            if(offset + index < (int)size) {
                fprintf(fp, "%02x ", 0xff & src[offset + index]);
            } else {
                fprintf(fp, "   ");
            }
        }
        fprintf(fp, "| ");
        for(index = 0; index < 16; index++) {
            if(offset + index < (int)size) {
                if(isascii(src[offset + index]) && isprint(src[offset + index])) {
                    fprintf(fp, "%c", src[offset + index]);
                } else {
                    fprintf(fp, ".");
                }
            } else {
                fprintf(fp, " ");
            }
        }
        fprintf(fp, " |\n");
    }
    fprintf(fp, "+------+-------------------------------------------------+------------------+\n");
}

uint16_t
cksum16 (uint16_t *data, uint16_t size, uint32_t init) {
    uint32_t sum;

    sum = init;
    while(size > 1) {
        sum += *(data++);
        size -= 2;
    }
    if(size) {
        sum += *(uint8_t *)data;
    }
    sum  = (sum & 0xffff) + (sum >> 16);
    sum  = (sum & 0xffff) + (sum >> 16);
    return ~(uint16_t)sum;
}

struct queue_entry *
queue_push (struct queue_head *queue, void *data, size_t size) {
    struct queue_entry *entry;

    if (!queue || !data) {
        return NULL;
    }
    entry = malloc(sizeof(struct queue_entry));
    if (!entry) {
        return NULL;
    }
    entry->data = data;
    entry->size = size;
    entry->next = NULL;
    if (queue->tail) {
        queue->tail->next = entry;
    }
    queue->tail = entry;
    if (!queue->next) {
        queue->next = entry;
    }
    queue->num++;
    return entry;
}

struct queue_entry *
queue_pop (struct queue_head *queue) {
    struct queue_entry *entry;

    if (!queue || !queue->next) {
        return NULL;
    }
    entry = queue->next;
    queue->next = entry->next;
    if (!queue->next) {
        queue->tail = NULL;
    }
    queue->num--;
    return entry;
}

uint16_t
hton16 (uint16_t h) {
    return htons(h);
}

uint16_t
ntoh16 (uint16_t n) {
    return ntohs(n);
}

uint32_t
hton32 (uint32_t h) {
    return htonl(h);
}

uint32_t
ntoh32 (uint32_t n) {
    return ntohl(n);
}

void
maskset (uint32_t *mask, size_t size, size_t offset, size_t len) {
    size_t idx, so, sb, bl;

    so = offset / 32;
    sb = offset % 32;
    bl = (len > 32 - sb) ? 32 - sb : len;
    mask[so] |= (0xffffffff >> (32 - bl)) << sb;
    len -= bl;
    for (idx = so; idx < so + (len / 32); idx++) {
        mask[idx + 1] = 0xffffffff;
    }
    len -= (32 * idx);
    if (len) {
        mask[idx + 1] |= (0xffffffff >> (32 - len));
    }
}

int
maskchk (uint32_t *mask, size_t size, size_t offset, size_t len) {
    size_t idx, so, sb, bl;

    so = offset / 32;
    sb = offset % 32;
    bl = (len > 32 - sb) ? 32 - sb : len;
    if (mask[offset / 32] & ((0xffffffff >> (32 - bl)) << sb) ^ ((0xffffffff >> (32 - bl)) << sb)) {
        return 0;
    }
    len -= bl;
    for (idx = so; idx < so + (len / 32); idx++) {
        if (mask[idx + 1] ^ 0xffffffff) {
            return 0;
        }
    }
    len -= (32 * idx);
    if (len) {
        if (mask[idx + 1] & (0xffffffff >> (32 - len)) ^ (0xffffffff >> (32 - len))) {
            return 0;
        }
    }
    return 1;
}

void
maskclr (uint32_t *mask, size_t size) {
    memset(mask, 0, sizeof(*mask) * size);
}

#define ISBIT(x) (x ? 1 : 0)
void
maskdbg (void *mask, size_t size) {
    uint8_t *ptr;

    for (ptr = (uint8_t *)mask; ptr < (uint8_t *)mask + size; ptr++) {
        fprintf(stderr, "%d%d%d%d %d%d%d%d\n",
            ISBIT(*ptr & 0x01), ISBIT(*ptr & 0x02), ISBIT(*ptr & 0x04), ISBIT(*ptr &0x08),
            ISBIT(*ptr & 0x10), ISBIT(*ptr & 0x20), ISBIT(*ptr & 0x40), ISBIT(*ptr &0x80));
    }
}
/*
void
bitmap_set_bit (uint32_t *bitmap, size_t size, size_t offset, size_t len) {
    size_t index, pos, bit;

    if (offset + len > size * 32) {
        return;
    }
    while (len) {
        index = offset / 32;
        pos = offset % 32;
        bit = (len < (32 - pos)) ? len : (32 - pos);
        bitmap[index] = (0xffffffff >> pos) & (0xffffffff << 32 - (pos + bit));
        len -= bit;
        offset += bit;
    }
}
*/

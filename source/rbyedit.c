#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define assert(expr) do{if(!(expr)) *(char *)0 = 0;} while(0)

#define CHECKSUM_ADDR 0x3523
#define CHECKSUM_START_ADDR 0x2598
#define CHECKSUM_END_ADDR 0x3522

#define PLAYER_NAME_ADDR 0x2598
#define ENEMY_NAME_ADDR 0x25F6
#define MONEY_ADDR 0x25F3
#define BAG_ADDR 0x25C9

typedef struct
{
	int id;
	int count;
} ListEntry;

typedef struct
{
	int count;
	ListEntry *entries;
} List;

typedef struct
{
	char *player_name;
	char *rival_name;
	int money;
	List bag;
} SaveData;

uint8_t ascii_to_rby(uint8_t c)
{
	if((c >= 'A' && c <= 'Z') ||
   	   (c >= 'a' && c <= 'z'))
	{
		return c + 63;
	}

	return c;
}

uint8_t rby_to_ascii(uint8_t c)
{
	if((c >= 0x80 && c <= 0x99) ||
   	   (c >= 0xA0 && c <= 0xB9))
	{
		return c - 63;
	}

	return c;
}

List get_bag(uint8_t *save)
{
	List list;
	list.count = save[BAG_ADDR];
	list.entries = malloc(20 * sizeof(ListEntry));

	int addr = BAG_ADDR + 1;
	for(int i = 0; i < list.count; i++)
	{
		ListEntry entry;
		entry.id = save[addr];
		entry.count = save[addr + 1];
		list.entries[i] = entry;
		addr += 2;
	}

	return list;
}

int get_money(uint8_t *save)
{
	uint32_t bcd = *(uint32_t *)(save + MONEY_ADDR);
	bcd = __builtin_bswap32(bcd);
	bcd = bcd >> 8;
	
	int money = 0;
	int multiplier = 1;

	while(bcd > 0)
	{
		money += multiplier * (bcd & 0x0F);
		bcd = bcd >> 4;
		multiplier *= 10;
	}

	return money;
}

char *get_rival_name(uint8_t *save)
{
	char *name = malloc(8);
	assert(name);

	int i;
	for(i = 0; save[ENEMY_NAME_ADDR+ i] != 0x50; i++)
	{
		name[i] = rby_to_ascii(save[ENEMY_NAME_ADDR + i]);
	}
	name[i] = '\0';

	return name;

}

char *get_player_name(uint8_t *save)
{
	char *name = malloc(8);
	assert(name);

	int i;
	for(i = 0; save[PLAYER_NAME_ADDR + i] != 0x50; i++)
	{
		name[i] = rby_to_ascii(save[PLAYER_NAME_ADDR + i]);
	}
	name[i] = '\0';

	return name;
}



void set_checksum(uint8_t *save)
{
	uint8_t checksum = 0;

	for(int addr = CHECKSUM_START_ADDR; addr <= CHECKSUM_END_ADDR; addr++)
	{
		checksum += save[addr];
	}

	save[CHECKSUM_ADDR] = ~checksum;
}

void set_bag(uint8_t *save, List bag)
{
	save[BAG_ADDR] = bag.count;
	int addr = BAG_ADDR + 1;

	if(bag.count == 0)
	{
		save[addr] = 0xFF;
	}
	else
	{
		for(int i = 0; i < bag.count; i++)
		{
			ListEntry entry = bag.entries[i];
			save[addr] = entry.id;
			save[addr + 1] = entry.count;
			addr += 2;
		}

		save[BAG_ADDR + (2 * bag.count) + 1] = 0xFF;
	}
}

void set_money(uint8_t *save, uint32_t amount)
{
	if(amount > 999999) amount = 999999;
	
	uint32_t bcd = 0;
	int shift = 0;
	while(amount > 0)
	{
		bcd |= (amount % 10) << shift;
		amount /= 10;
		shift += 4;
	}

	save[MONEY_ADDR] = bcd >> 16;
	save[MONEY_ADDR + 1] = bcd >> 8;
	save[MONEY_ADDR + 2] = bcd;
}

void write_string(uint8_t *save, int address, char *string)
{
	int i;
	for(i = 0; string[i] != '\0'; i++)
	{
		save[address + i] = ascii_to_rby(string[i]);
	}

	save[address + i] = 0x50;
}

void update_save(uint8_t *save, SaveData save_data)
{
	set_money(save, save_data.money);
	write_string(save, PLAYER_NAME_ADDR, save_data.player_name);
	write_string(save, ENEMY_NAME_ADDR, save_data.rival_name);
	set_bag(save, save_data.bag);
	set_checksum(save);
}

SaveData get_save_data(uint8_t *save)
{
	SaveData save_data;
	
	save_data.rival_name = get_rival_name(save);
	save_data.player_name = get_player_name(save);
	save_data.money = get_money(save);
	save_data.bag = get_bag(save);

	return save_data;
}

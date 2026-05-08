#include <stdio.h>
#include <iup.h>
#include "rbyedit.c"

Ihandle *main_dlg, *money_text, *player_name_text, *rival_name_text, *bag_vbox;
uint8_t *save;
SaveData save_data;
FILE *file;
int file_size;

Ihandle *create_list_entry_hbox(ListEntry entry)
{
	char buffer[8];
	Ihandle *id_label, *id_text, *count_label, *count_text, *entry_hbox;

	id_label = IupLabel("ID");
	count_label = IupLabel("Count");

	id_text = IupText(0);
	count_text = IupText(0);

	snprintf(buffer, sizeof(buffer), "%d", entry.id);
	IupStoreAttribute(id_text, "VALUE", buffer);

	snprintf(buffer, sizeof(buffer), "%d", entry.count);
	IupStoreAttribute(count_text, "VALUE", buffer);

	entry_hbox = IupHbox(id_label, id_text, count_label, count_text, 0);

	return entry_hbox;
}

int open_file(Ihandle *self)
{
	if(file)
	{
		fclose(file);
	}

	Ihandle *file_dlg = IupFileDlg();
	IupPopup(file_dlg, IUP_CENTER, IUP_CENTER);

	file = fopen(IupGetAttribute(file_dlg, "VALUE"), "rb+");
	if(!file)
	{
		IupMessage("error", "could not find file");
		return IUP_DEFAULT;
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	save = malloc(file_size);
	if(!save)
	{
		fclose(file);
		IupMessage("error", "could not allocate memory for file");
		return IUP_DEFAULT;
	}

	fread(save, 1, file_size, file);

	save_data = get_save_data(save);

	IupSetAttribute(player_name_text, "VALUE", save_data.player_name);
	IupSetAttribute(rival_name_text, "VALUE", save_data.rival_name);

	for(int i = 0; i < save_data.bag.count; i++)
	{
		Ihandle *entry_hbox = create_list_entry_hbox(save_data.bag.entries[i]);
		IupAppend(bag_vbox, entry_hbox);
		IupMap(entry_hbox);
	}
	IupRefresh(bag_vbox);


	char buffer[7];
	snprintf(buffer, sizeof(buffer), "%d", save_data.money);
	IupSetAttribute(money_text, "VALUE", buffer);

	return IUP_DEFAULT;
}

int write_save(Ihandle *self)
{
	assert(file);

	save_data.player_name = IupGetAttribute(player_name_text, "VALUE");
	save_data.rival_name = IupGetAttribute(rival_name_text, "VALUE");
	save_data.money = atoi(IupGetAttribute(money_text, "VALUE"));

	for(int i = 0; i < save_data.bag.count; i++)
	{
		Ihandle *entry_hbox, *id_text, *count_text;
	   	entry_hbox = IupGetChild(bag_vbox, i);
		id_text = IupGetChild(entry_hbox, 1);
		count_text = IupGetChild(entry_hbox, 3);

		ListEntry entry;
		entry.id = atoi(IupGetAttribute(id_text, "VALUE"));
		entry.count = atoi(IupGetAttribute(count_text, "VALUE"));
		save_data.bag.entries[i] = entry;
	}

	update_save(save, save_data);
	fseek(file, 0, SEEK_SET);
	fwrite(save, 1, file_size, file);
	IupMessage("saved", "applied changes to save file!");
	return IUP_DEFAULT;
}

Ihandle *create_main_dialog(void)
{
	Ihandle *dlg, *vbox;
	Ihandle *open_button, *save_button, *save_hbox;
	Ihandle *player_name_label, *player_name_hbox;
	Ihandle *rival_name_label, *rival_name_hbox;
	Ihandle *money_label, *money_hbox;

	open_button = IupButton("Open Save", 0);
	save_button = IupButton("Save Changes", 0);
	IupSetCallback(open_button, "ACTION", (Icallback) open_file);
	IupSetCallback(save_button, "ACTION", (Icallback) write_save);
	save_hbox = IupHbox(open_button, save_button, 0);

	player_name_label = IupLabel("Player Name");
	player_name_text = IupText(0);
	player_name_hbox = IupHbox(player_name_label, player_name_text);

	rival_name_label = IupLabel("Rival Name");
	rival_name_text = IupText(0);
	rival_name_hbox = IupHbox(rival_name_label, rival_name_text);

	money_label = IupLabel("Money");
	money_text = IupText(0);
	money_hbox = IupHbox(money_label, money_text, 0);

	bag_vbox = IupVbox(0);
	
	vbox = IupVbox(
			save_hbox, player_name_hbox, rival_name_hbox, money_hbox, bag_vbox, 0);

	dlg = IupDialog(vbox);
	IupSetAttribute(dlg, "TITLE", "RBY Edit");
	return dlg;
}

int main(int argc, char **args)
{
	IupOpen(&argc, &args);

	Ihandle *dlg = create_main_dialog();
	IupShowXY(dlg, IUP_CENTER, IUP_CENTER);
	IupMainLoop();
}

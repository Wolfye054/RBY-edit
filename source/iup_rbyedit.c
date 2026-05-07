#include <stdio.h>
#include <iup.h>
#include "rbyedit.c"

Ihandle *main_dlg, *money_text, *player_name_text;
uint8_t *save;
SaveData save_data;
FILE *file;
int file_size;

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

	char buffer[7];
	snprintf(buffer, sizeof(buffer), "%d", save_data.money);
	IupSetAttribute(money_text, "VALUE", buffer);

	return IUP_DEFAULT;
}

int write_save(Ihandle *self)
{
	assert(file);

	save_data.player_name = IupGetAttribute(player_name_text, "VALUE");
	save_data.money = atoi(IupGetAttribute(money_text, "VALUE"));

	update_save(save, save_data);
	fseek(file, 0, SEEK_SET);
	fwrite(save, 1, file_size, file);
	return IUP_DEFAULT;
}

Ihandle *create_main_dialog(void)
{
	Ihandle *dlg, *vbox;
	Ihandle *open_button, *save_button, *save_hbox;
	Ihandle *player_name_label, *player_name_hbox;
	Ihandle *money_label, *money_hbox;

	open_button = IupButton("Open Save", 0);
	save_button = IupButton("Save Changes", 0);
	IupSetCallback(open_button, "ACTION", (Icallback) open_file);
	IupSetCallback(save_button, "ACTION", (Icallback) write_save);
	save_hbox = IupHbox(open_button, save_button, 0);

	player_name_label = IupLabel("Player Name");
	player_name_text = IupText(0);
	player_name_hbox = IupHbox(player_name_label, player_name_text);

	money_label = IupLabel("Money");
	money_text = IupText(0);
	money_hbox = IupHbox(money_label, money_text, 0);
	
	vbox = IupVbox(
			save_hbox, player_name_hbox, money_hbox, 0);

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

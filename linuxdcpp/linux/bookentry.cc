#include "bookentry.hh"

using namespace std;

BookEntry::BookEntry(int type, string id, string title, GCallback closeCallback) {
	this->type = type;
	this->id = id;
	
	label = GTK_LABEL(gtk_label_new(title.c_str()));
	button = GTK_BUTTON(gtk_button_new_from_stock(GTK_STOCK_CLOSE));
	box = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(label), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(button), FALSE, TRUE, 0);
	gtk_widget_show_all(box);
	
    g_signal_connect(G_OBJECT(button), "clicked", 
		closeCallback, (gpointer)this);
}

BookEntry::~BookEntry() {
}

GtkWidget *BookEntry::getTitle() {
	return box;
}

void BookEntry::setLabel_gui(std::string text) {
	gtk_label_set_text(label, text.c_str());
}

bool BookEntry::isEqual(int type, string id) {
	if (this->type == type) 
		return this->id == id;
	else 
		return false;
}

bool BookEntry::isEqual(BookEntry *entry) {
	if (this->type == entry->type) 
		return this->id == entry->id;
	else 
		return false;
}

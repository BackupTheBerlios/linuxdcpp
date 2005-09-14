#ifndef WULFOR_FINISHED_TRANSFERS
#define WULFOR_FINISHED_TRANSFERS

#include "bookentry.hh"
#include "wulformanager.hh"
#include "callback.hh"
#include <client/FinishedManager.h>

class FinishedTransfers:
	public BookEntry,
	public FinishedManagerListener
{
	public:
	
		FinishedTransfers(int type, std::string title, GCallback closeCallback);
		~FinishedTransfers();
	
		GtkWidget *getWidget();
	
		static void popupMenu(GtkWidget *, GdkEventButton *, gpointer);
		static void removeItems(GtkMenuItem *, gpointer);
		static void removeAll(GtkMenuItem *, gpointer);
		static void openWith(GtkMenuItem *, gpointer);

		void updateList(FinishedItem::List& list);
		void addEntry(FinishedItem *entry);
		void updateStatus();
	
		//from FinishedManagerListener
		void on(AddedDl, FinishedItem* entry) throw();
		void on(AddedUl, FinishedItem* entry) throw();
	
	private:
	
		GtkWidget *mainBox;
		GtkListStore *transferStore;
		GtkTreeView *transferView;
		GtkTreeIter treeIter;
		GtkStatusbar *totalItems, *totalSize, *averageSpeed;
		GtkTreeSelection *transferSelection;
		GtkDialog *openDialog;
		GtkEntry *openEntry;
	
		int items;
		bool isUpload;
		int64_t totalBytes, totalTime;
	
		GtkMenu *finishedTransfersMenu;
		GtkMenuItem *openItem, *removeItem, *removeAllItem;
	
		enum
		{
			COLUMN_TIME,
			COLUMN_FILENAME,
			COLUMN_PATH,
			COLUMN_NICK,
			COLUMN_HUB,
			COLUMN_SIZE,
			COLUMN_SPEED,
			COLUMN_CRC,
			COLUMN_TARGET
		};
	
};
#else
class FinishedTransfers;
#endif

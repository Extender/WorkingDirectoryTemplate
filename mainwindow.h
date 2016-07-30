#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <vector>
#include <list>
#include <ctime>

#include "text.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class ExampleItemClass
{
public:
    QString name;
    QString value;

    ExampleItemClass()
    {
        name="";
        value="";
    }

    ExampleItemClass(QString _name,QString _value)
    {
        name=_name;
        value=_value;
    }

    ~ExampleItemClass()
    {
    }
};

// Define item type here:
typedef ExampleItemClass new_item_t;

typedef new_item_t* item_t;

// Define item extension here:
#define ITEM_EXTENSION (".itm")

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString workingDir;
    QFileDialog *workingDirDialog;
    QStringList *defaultFilters;
    int selectedIndex;
    item_t selectedItem;

    vector<item_t> *items;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString getUniqueItemName();

public slots:
    void browseWorkingDirectoryBtnClicked();
    void setWorkingDirectoryBtnClicked();
    void workingDirDialogFileSelected(QString newPath);
    void unselectItem();
    void selectItem(int index);
    void itemListSelectionChanged();
    void itemNameBoxEditingFinished();
    void itemValueBoxEditingFinished();
    void newItemBtnClicked();
    void deleteItemBtnClicked();
    void duplicateItemBtnClicked();
    void filterBtnClicked();
    void resetFilterBtnClicked();
    void saveCurrentItem();
    void addNewItem(QString name,QString value);
    void filterItems(QString wildcard);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

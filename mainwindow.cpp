#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    srand(time(0));

    ui->setupUi(this);

    items=new vector<item_t>();
    selectedIndex=0; // Needed before first call to unselectItem()
    unselectItem();

    defaultFilters=new QStringList();
    QString ext=ITEM_EXTENSION;
    defaultFilters->append("*"+ext);

    QString exampleDirPath=QApplication::applicationDirPath()+QString("/ExampleDir/");
    QDir d(exampleDirPath);
    if(!d.exists())
        QDir().mkdir(exampleDirPath);

    workingDir="";
    workingDirDialog=new QFileDialog(this);
    workingDirDialog->setAcceptMode(QFileDialog::AcceptOpen);
    workingDirDialog->setFileMode(QFileDialog::Directory);
    workingDirDialog->setOption(QFileDialog::ShowDirsOnly);
    workingDirDialog->setWindowTitle("Select a directory...");
    connect(workingDirDialog,SIGNAL(fileSelected(QString)),this,SLOT(workingDirDialogFileSelected(QString)));

    connect(ui->browseWorkingDirectoryBtn,SIGNAL(clicked(bool)),this,SLOT(browseWorkingDirectoryBtnClicked()));
    connect(ui->setWorkingDirectoryBtn,SIGNAL(clicked(bool)),this,SLOT(setWorkingDirectoryBtnClicked()));

    connect(ui->itemList,SIGNAL(itemSelectionChanged()),this,SLOT(itemListSelectionChanged()));
    connect(ui->itemNameBox,SIGNAL(editingFinished()),this,SLOT(itemNameBoxEditingFinished()));
    connect(ui->itemValueBox,SIGNAL(editingFinished()),this,SLOT(itemValueBoxEditingFinished()));

    connect(ui->newItemBtn,SIGNAL(clicked(bool)),this,SLOT(newItemBtnClicked()));
    connect(ui->deleteItemBtn,SIGNAL(clicked(bool)),this,SLOT(deleteItemBtnClicked()));
    connect(ui->duplicateItemBtn,SIGNAL(clicked(bool)),this,SLOT(duplicateItemBtnClicked()));

    connect(ui->filterBtn,SIGNAL(clicked(bool)),this,SLOT(filterBtnClicked()));
    connect(ui->resetFilterBtn,SIGNAL(clicked(bool)),this,SLOT(resetFilterBtnClicked()));

    ui->workingDirectoryBox->setText(exampleDirPath);
    setWorkingDirectoryBtnClicked();
}

MainWindow::~MainWindow()
{
    delete ui;
    int itemCount=items->size();
    for(int i=0;i<itemCount;i++)
        delete items->at(i);
    delete items;
    delete defaultFilters;
    delete workingDirDialog;
}

QString MainWindow::getUniqueItemName()
{
    QString pathTemplate=workingDir+QString("Item #%r%")+QString(ITEM_EXTENSION);
    QString path;
    QString rNum;
    Loop:
    {
        rNum=QString::number(rand());
        path=pathTemplate.replace("%r%",rNum);
        QFile f(path);
        if(f.exists())
            goto Loop;
    }
    return QString("Item #")+rNum;
}

void MainWindow::browseWorkingDirectoryBtnClicked()
{
    workingDirDialog->exec();
}

void MainWindow::setWorkingDirectoryBtnClicked()
{
    QString path=ui->workingDirectoryBox->text();
    if(path.length()==0)
    {
        QMessageBox::critical(this,"Error","No directory specified.");
        ui->workingDirectoryBox->setText(workingDir);
        return;
    }
    QDir d(path);
    if(!d.exists())
    {
        QMessageBox::critical(this,"Error","The specified directory does not exist.");
        ui->workingDirectoryBox->setText(workingDir);
        return;
    }

    path=path.replace("\\","/");
    if(!path.endsWith('/'))
        path+='/';

    workingDir=path;
    unselectItem();
    ui->itemList->clear();

    // Enumerate items in dir

    QFileInfoList l=d.entryInfoList(*defaultFilters,QDir::Files);
    for(QFileInfo i:l)
    {
        ui->itemList->addItem(i.baseName());
        QFile f(i.absoluteFilePath());
        f.open(QFile::ReadOnly);
        QString fileData=QString(f.readAll());
        f.close();
        QStringList parts=fileData.split("\n");
        QString name=parts.at(0);
        QString value=parts.at(1);
        items->push_back(new new_item_t(name,value));
    }
}

void MainWindow::workingDirDialogFileSelected(QString newPath)
{
    ui->workingDirectoryBox->setText(newPath);
}

void MainWindow::unselectItem()
{
    if(selectedIndex==-1)
        return;

    selectedIndex=-1;
    selectedItem=0;
    ui->itemNameBox->setText("");
    ui->itemValueBox->setText("");
    list<QWidget*> frameChildren=ui->itemFrame->findChildren<QWidget*>().toStdList();
    for_each(frameChildren.begin(),frameChildren.end(),[](QWidget *w){w->setVisible(false);});
}

void MainWindow::selectItem(int index)
{
    selectedIndex=index;
    selectedItem=items->at(index);
    ui->itemNameBox->setText(selectedItem->name);
    ui->itemValueBox->setText(selectedItem->value);
    list<QWidget*> frameChildren=ui->itemFrame->findChildren<QWidget*>().toStdList();
    for_each(frameChildren.begin(),frameChildren.end(),[](QWidget *w){w->setVisible(true);});
}

void MainWindow::itemListSelectionChanged()
{
    QList<QListWidgetItem*> selectedItems=ui->itemList->selectedItems();
    if(selectedItems.count()==0)
    {
        unselectItem();
        return;
    }

    // Filter may be enabled; find index by name

    QString name=selectedItems.at(0)->text();
    int itemCount=items->size();
    int index=-1;
    for(int i=0;i<itemCount;i++)
    {
        if(name==items->at(i)->name)
        {
            index=i;
            break;
        }
    }
    selectItem(index);
}

void MainWindow::itemNameBoxEditingFinished()
{
    if(selectedIndex==-1)
        return;

    QString n=ui->itemNameBox->text();
    if(n==selectedItem->name)
        return;

    // Check if name already taken

    int itemCount=items->size();
    for(int i=0;i<itemCount;i++)
    {
        if(items->at(i)->name==n)
        {
            QMessageBox::critical(this,"Error","Name already taken.");
            ui->itemNameBox->setText(n);
            return;
        }
    }

    // Delete file with old name

    QFile::remove(workingDir+selectedItem->name+QString(ITEM_EXTENSION));

    selectedItem->name=n;
    ui->itemList->selectedItems().at(0)->setText(n);
    saveCurrentItem();
}

void MainWindow::itemValueBoxEditingFinished()
{
    if(selectedIndex==-1)
        return;
    QString n=ui->itemValueBox->text();
    selectedItem->value=n;
    saveCurrentItem();
}

void MainWindow::newItemBtnClicked()
{
    if(workingDir.length()==0)
    {
        QMessageBox::critical(this,"Error","No working directory specified.");
        return;
    }

    addNewItem(getUniqueItemName(),"");
}

void MainWindow::deleteItemBtnClicked()
{
    if(workingDir.length()==0)
    {
        QMessageBox::critical(this,"Error","No working directory specified.");
        return;
    }

    items->erase(items->begin()+selectedIndex);
    ui->itemList->removeItemWidget(ui->itemList->selectedItems().at(0));
    unselectItem();
}

void MainWindow::duplicateItemBtnClicked()
{
    if(workingDir.length()==0)
    {
        QMessageBox::critical(this,"Error","No working directory specified.");
        return;
    }

    addNewItem(QString("Copy of ")+selectedItem->name,selectedItem->value);
}

void MainWindow::filterBtnClicked()
{
    filterItems(ui->filterBox->text());
}

void MainWindow::resetFilterBtnClicked()
{
    ui->filterBox->setText("");
    filterItems("");
}

void MainWindow::saveCurrentItem()
{
    QFile f(workingDir+selectedItem->name+QString(ITEM_EXTENSION));
    f.open(QFile::WriteOnly|QFile::Truncate);
    QString out=selectedItem->name+QString("\n")+selectedItem->value;
    f.write(QByteArray(out.toStdString().c_str()));
    f.close();
}

void MainWindow::addNewItem(QString name, QString value)
{
    item_t item=new new_item_t(name,value);

    items->push_back(item);
    QListWidgetItem *newItem=new QListWidgetItem(name,ui->itemList);
    ui->itemList->addItem(newItem);
    ui->itemList->clearSelection();
    newItem->setSelected(true);

    // Create file
    saveCurrentItem();
}

void MainWindow::filterItems(QString wildcard)
{
    unselectItem();
    ui->itemList->clear();
    int itemCount=items->size();
    if(wildcard.length()==0)
    {
        for(int i=0;i<itemCount;i++)
            ui->itemList->addItem(items->at(i)->name);
        return;
    }

    char *pattern=text::duplicateString((QString("*")+QString(wildcard)+QString("*")).toStdString().c_str()); // Must be duplicated
    for(int i=0;i<itemCount;i++)
    {
        item_t item=items->at(i);
        char *name=strdup(item->name.toStdString().c_str()); // Must be duplicated
        if(text::matchWildcard(name,pattern,true))
            ui->itemList->addItem(item->name);
        free(name);
    }
    free(pattern);
}

#include <QDebug>
#include <QStandardItemModel>

#include "connecteditor.h"
#include "schemadefinitionmodel.h"
#include "theme.h"
#include "ui_connecteditor.h"

namespace gams {
namespace studio {
namespace connect {

ConnectEditor::ConnectEditor(const QString& connectDataFileName, QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ConnectEditor),
    mLocation(connectDataFileName)
{
    init();
}

bool ConnectEditor::init()
{
    ui->setupUi(this);

    qDebug() << "ConnectEditor::" << mLocation;

    mConnect = new Connect();
    QStringList schema = mConnect->getSchemaNames();

    QStandardItemModel* schemaItemModel = new QStandardItemModel( mConnect->getSchemaNames().size(), 1, this );
    QStandardItemModel* schemaHelpModel = new QStandardItemModel( mConnect->getSchemaNames().size(), 1, this );

    for(int row=0; row<mConnect->getSchemaNames().size(); row++) {
        QString str = mConnect->getSchemaNames().at(row);
        QStandardItem *helpitem = new QStandardItem();
        helpitem->setData( str, Qt::DisplayRole );
        schemaHelpModel->setItem(row, 0, helpitem);

        QStandardItem *item = new QStandardItem();
        item->setData( str, Qt::DisplayRole );
        item->setIcon(Theme::icon(":/%1/plus", false));
        item->setEditable(false);
        item->setSelectable(true);
        item->setTextAlignment(Qt::AlignLeft);
        schemaItemModel->setItem(row, 0, item);
    }

    ui->SchemaControlListView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->SchemaControlListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->SchemaControlListView->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->SchemaControlListView->setViewMode(QListView::ListMode);
    ui->SchemaControlListView->setIconSize(QSize(16,16));
    ui->ConnectHSplitter->setSizes(QList<int>({10, 70, 20}));
    ui->SchemaControlListView->setModel(schemaItemModel);

    ui->helpComboBox->setModel(schemaHelpModel);

    SchemaDefinitionModel* defmodel = new SchemaDefinitionModel(mConnect, mConnect->getSchemaNames().first(), this);
    ui->helpTreeView->setModel( defmodel );
    ui->helpTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->helpTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->helpTreeView->resizeColumnToContents(0);
    ui->helpTreeView->resizeColumnToContents(1);
    ui->helpTreeView->resizeColumnToContents(2);
    ui->helpTreeView->setItemsExpandable(true);
    headerRegister(ui->helpTreeView->header());

    connect(ui->SchemaControlListView, &QListView::clicked, this, &ConnectEditor::schemaClicked);
    connect(ui->SchemaControlListView, &QListView::doubleClicked, this, &ConnectEditor::schemaDoubleClicked);

    connect(ui->helpComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        defmodel->loadSchemaFromName( schemaHelpModel->data( schemaHelpModel->index(index,0) ).toString() );
    });

    return true;
}

ConnectEditor::~ConnectEditor()
{
    delete ui;
    if (mConnect)
        delete mConnect;
}

void ConnectEditor::schemaClicked(const QModelIndex &modelIndex)
{
    qDebug() << "clikced row=" << modelIndex.row() << ", col=" << modelIndex.column();
}

void ConnectEditor::schemaDoubleClicked(const QModelIndex &modelIndex)
{
    qDebug() << "doubleclikced row=" << modelIndex.row() << ", col=" << modelIndex.column();
}

}
}
}

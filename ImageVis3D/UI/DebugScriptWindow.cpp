/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2012 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

/**
 \brief Combination of the debug window and a new scripting window.
 */


#include <QtGui/QVBoxLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QListWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QEvent>
#include <QKeyEvent>

#include "DebugScriptWindow.h"

//-----------------------------------------------------------------------------
DebugScriptWindow::DebugScriptWindow(tuvok::MasterController& controller,
                                     QWidget* parent)
: QDockWidget("Debug / Scripting Window", parent)
, mController(controller)
, mMemReg(controller.LuaScript())
, mLua(controller.LuaScript())
, mSavedInputPos(0)
{
  setupUI();
  hookLuaFunctions();

  mSavedInput.reserve(50);

  // Install an event filter for our line edit
  mScriptOneLineEdit->installEventFilter(this);
}

//-----------------------------------------------------------------------------
DebugScriptWindow::~DebugScriptWindow()
{

}

//-----------------------------------------------------------------------------
void DebugScriptWindow::setupUI()
{
  // Testing setting up the UI programatically, as opposed to using QTCreator
  // and its generated XML.

  QWidget* dockWidgetContents = new QWidget();
  dockWidgetContents->setObjectName(QString::fromUtf8("DebugWindowContents"));
  this->setWidget(dockWidgetContents);

  mMainLayout = new QVBoxLayout(dockWidgetContents);
  mMainLayout->setSpacing(6);
  mMainLayout->setContentsMargins(9, 9, 9, 9);
  mMainLayout->setObjectName(QString::fromUtf8("verticalLayout"));

  mTabWidget = new QTabWidget;
  mMainLayout->addWidget(mTabWidget);

  // Script debug implementation.
  {
    QWidget* scriptTabContents = new QWidget();
    mTabWidget->addTab(scriptTabContents, QString::fromUtf8("script"));

    QHBoxLayout* scriptLayout = new QHBoxLayout(scriptTabContents);
    scriptTabContents->setLayout(scriptLayout);

    // Output / one line interaction
    {
      QWidget* outputContents = new QWidget();
      scriptLayout->addWidget(outputContents);

      QVBoxLayout* outputLayout = new QVBoxLayout();
      outputContents->setLayout(outputLayout);

      mListWidget = new QListWidget();
      outputLayout->addWidget(mListWidget);

      // Lower line edit and label.
      {
        QWidget* cont = new QWidget();
        cont->setMinimumHeight(0);
        outputLayout->addWidget(cont);

        QHBoxLayout* hboxLayout = new QHBoxLayout();
        cont->setLayout(hboxLayout);

        QLabel* lbl = new QLabel();
        lbl->setText(QString::fromUtf8("Command: "));
        lbl->setMinimumSize(QSize(0, 0));
        hboxLayout->addWidget(lbl);

        mScriptOneLineEdit = new QLineEdit();
        hboxLayout->addWidget(mScriptOneLineEdit);
        QObject::connect(mScriptOneLineEdit, SIGNAL(returnPressed()), this,
                         SLOT(oneLineEditOnReturnPressed()));
        QObject::connect(mScriptOneLineEdit, SIGNAL(textEdited()), this,
                         SLOT(oneLineEditOnEdited()));
      }
    }

    // Script interaction.
    {
      QWidget* editContents = new QWidget();
      scriptLayout->addWidget(editContents);

      QVBoxLayout* editLayout = new QVBoxLayout();
      editContents->setLayout(editLayout);

      mScriptTextEdit = new QTextEdit();
      editLayout->addWidget(mScriptTextEdit);

      // Combo box combined with execute button.
      {
        QWidget* comboExecContents = new QWidget();
        comboExecContents->setMinimumHeight(0);
        editLayout->addWidget(comboExecContents);

        QHBoxLayout* comboExecLayout = new QHBoxLayout();
        comboExecContents->setLayout(comboExecLayout);

        QLabel* lbl = new QLabel();
        lbl->setText(QString::fromUtf8("Examples: "));
        comboExecLayout->addWidget(lbl);

        mScriptExamplesBox = new QComboBox();
        comboExecLayout->addWidget(mScriptExamplesBox);
        QString emptyExample = QString::fromUtf8(" ");

        mScriptExamplesBox->addItem(QString::fromUtf8(" "),
                                    QVariant(emptyExample));

        QString regressionTesting = QString::fromUtf8(
            "-- Please modify the variables below to point to valid paths.\n"
            "local homeDir = os.getenv('HOME')\n"
            "regress_scriptDir = homeDir .. '/sci/imagevis3d/Tuvok/LuaScripting"
            "/Regression/iv3d'\n"
            "regress_c60Dir = homeDir .. '/sci/datasets/c60.uvf'\n"
            "regress_outputDir = homeDir .. '/sci/outputDatasets'\n"
            "luaVerboseMode(true)\n\n"
            "-- Todo: Switch to LuaFileSystem -- cross platform\n"
            "for fname in dir(regress_scriptDir) do\n"
            "  if fname ~= '.' and fname ~= '..' then\n"
            "    print('Running \\'' .. fname .. '\\'')\n"
            "    dofile(regress_scriptDir .. '/' .. fname)\n"
            "  end\n"
            "end\n\n"
            "luaVerboseMode(false)\n");
        mScriptExamplesBox->addItem(QString::fromUtf8("Regression Testing"),
                                    QVariant(regressionTesting));

        QString autoScreenCap = QString::fromUtf8(
            "-- This script will only run on a Posix compliant OS.\n"
            "-- Opens the dataset given by 'filename' and begins taking screen captures.\n"
            "-- You can manually transform the dataset while the script is running.\n"
            "-- Expect horrible things to happen if you close the render window while\n"
            "-- the script is running.\n"
            "local homeDir = os.getenv('HOME')\n"
            "local filename = homeDir .. '/sci/datasets/c60.uvf'\n"
            "local outputDir = homeDir .. '/sci/imageOutput'\n"
            "local numScreenCaps = 128\n\n"
            "print('Animate and capture script')\n"
            "print('Results will be output to: ' .. outputDir)\n"
            "os.execute('mkdir -p ' .. outputDir)\n\n"
            "-- data = Render window to animate.\n"
            "-- numFrames = Number of frames to capture during the animation.\n"
            "function doAnim (data, numFrames)\n"
            "  local datasetPath = data.getDataset().path()\n"
            "  local baseName = os.capture('basename ' .. datasetPath .. ' .uvf', false)\n"
            "  print('Using basename: ' .. baseName)\n"
            "  t = 0.0\n"
            "  dt = 2.0 * math.pi / numFrames\n"
            "  for i=1,numFrames do\n"
            "    t = t + dt\n"
            "    print('Capturing screen ' .. i)\n"
            "    data.screenCapture(outputDir .. '/' .. baseName .. '_' .. i .. '.png', false)\n"
            "    -- Dirty hack to get the UI to update.\n"
            "    iv3d.processUI()\n"
            "  end\n"
            "end\n\n"
            "print('Loading data')\n"
            "data = iv3d.renderer.new(filename)\n"
            "print('Performing screen captures')\n"
            "data.setRendererTarget(1)\n"
            "doAnim(data, numScreenCaps)\n"
            "data.setRendererTarget(0)\n");
        mScriptExamplesBox->addItem(QString::fromUtf8("Auto Screen Cap"),
                                    QVariant(autoScreenCap));


        QString exampleMath = QString::fromUtf8(
            "print('-- Binary Operators --')\n"
            "print(5 + 79)\n"
            "print('Basic binary operators: ' .. (3 * 5 - 2) / 5 + 1 )\n"
            "print('Exponentiation: ' .. 3 ^ 5)\n"
            "a = 17; b = 5;\n"
            "print('Modulo: ' .. a % b)\n"
            "print('-- Relational Operators --')\n"
            "print('Is it equal?: ' .. tostring(a%b == a-math.floor(a/b)*b))\n"
            "print('a less than b?: ' .. tostring(a < b))\n"
            "print('a greater than b?: ' .. tostring(a > b))\n");
        mScriptExamplesBox->addItem(QString::fromUtf8("Basic Math"),
                                    QVariant(exampleMath));
        QString exampleLightOnAll = QString::fromUtf8(
            "for i in ...;\n"
            "...\n"
            "...\n"
            "...\n"
            "..");
        mScriptExamplesBox->addItem(QString::fromUtf8("Turn On All Lighting"),
                                    QVariant(exampleLightOnAll));
        QString rotate360AndScreenCap = QString::fromUtf8(
            "for i in ...;\n"
            "...\n"
            "...\n"
            "...\n"
            "..");
        mScriptExamplesBox->addItem(QString::fromUtf8("Rotate 360 and Screen "
                                                      "Cap"),
                                    QVariant(rotate360AndScreenCap));
        QObject::connect(mScriptExamplesBox, SIGNAL(currentIndexChanged(int)),
                         this, SLOT(exampComboIndexChanged(int)));

        QSpacerItem* spacer = new QSpacerItem(40, 10,
                                              QSizePolicy::Expanding,
                                              QSizePolicy::Preferred);
        comboExecLayout->addSpacerItem(spacer);

        mExecButton = new QPushButton();
        mExecButton->setMinimumSize(QSize(0, 23));  // Required, if not, button
        // is shifted downwards beyond its layout control.
        mExecButton->setText(QString::fromUtf8("Execute Script"));
        comboExecLayout->addWidget(mExecButton);
        QObject::connect(mExecButton, SIGNAL(clicked()), this,
                         SLOT(execClicked()));
      }
    }
  }

  // Debug output from the program.
  {
    QWidget* debugTabContents = new QWidget();
    mTabWidget->addTab(debugTabContents, QString::fromUtf8("debug"));
  }
}

//-----------------------------------------------------------------------------
bool DebugScriptWindow::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == mScriptOneLineEdit)
  {
    if (mSavedInput.size() == 0) return false;

    if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Up)
      {
        /// @todo Implement scrolling through prior command history.
        //mController.LuaScript()->exec("print('key up')");
        mScriptOneLineEdit->setText(
            QString::fromStdString(mSavedInput[mSavedInputPos]));
        if (mSavedInputPos > 0)
          --mSavedInputPos;
        return true;
      }
      else if (keyEvent->key() == Qt::Key_Down)
      {
        //mController.LuaScript()->exec("print('key down')");
        if (mSavedInputPos < (int)mSavedInput.size() - 1)
        {
          ++mSavedInputPos;
          mScriptOneLineEdit->setText(
              QString::fromStdString(mSavedInput[mSavedInputPos]));
        }
        else
        {
          mScriptOneLineEdit->setText(QString::fromUtf8(""));
        }

        return true;
      }
    }
    return false;
  }
  return QDockWidget::eventFilter(obj, event);
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::execClicked()
{
  QString qs = mScriptTextEdit->document()->toPlainText();
  execLua(qs.toStdString());
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::oneLineEditOnReturnPressed()
{
  QString qs = mScriptOneLineEdit->text();
  execLua(qs.toStdString());
  mSavedInput.push_back(qs.toStdString());
  mSavedInputPos = mSavedInput.size() - 1;
  mScriptOneLineEdit->setText(QString::fromUtf8(""));
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::oneLineEditOnEdited()
{
  // Reset the stack pointer
  mSavedInputPos = mSavedInput.size() - 1;
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::execLua(const std::string& cmd)
{
  mLua->setExpectedExceptionFlag(true);
  try
  {
    mLua->exec(cmd);
  }
  catch (tuvok::LuaError& e)
  {
    std::string error = "Lua Error: ";
    error += e.what();
    hook_logError(error);

    // Check to see if we want to tell the user to add the parenthesis at the
    // end of the function.
    std::string extraInfoWarn("attempt to call a string value");
    if (extraInfoWarn.compare(e.what()) == 0)
    {
      hook_logError("  Note: Fixing this error might be as simple as adding "
                    "parenthesis '()' to the end of your command.");
      hook_logError("  Lua will not execute functions unless they are qualified"
                    " by the suffix '()'");
      hook_logError("  Try help()");
    }
  }
  catch (std::exception& e)
  {
    std::string error = "Standard Exception: ";
    error += e.what();
    hook_logError(error);
  }
  catch (...)
  {
    hook_logError("Unknown exception occurred.");
  }
  mLua->setExpectedExceptionFlag(false);
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::exampComboIndexChanged(int index)
{
  QVariant text = mScriptExamplesBox->itemData(index);
  mScriptTextEdit->setText(text.toString());
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::hookLuaFunctions()
{
  mMemReg.strictHook(this, &DebugScriptWindow::hook_logInfo, "print");
  mMemReg.strictHook(this, &DebugScriptWindow::hook_logInfo, "log.info");
  mMemReg.strictHook(this, &DebugScriptWindow::hook_logWarning, "log.warn");
  mMemReg.strictHook(this, &DebugScriptWindow::hook_logError, "log.error");
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::hook_logInfo(std::string info)
{
  mListWidget->addItem(QString::fromStdString(info));
  mListWidget->scrollToBottom();
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::hook_logWarning(std::string warn)
{
  QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(warn));
  item->setBackground(QBrush(QColor(246, 234, 190)));
  mListWidget->addItem(item);
  mListWidget->scrollToBottom();
}

//-----------------------------------------------------------------------------
void DebugScriptWindow::hook_logError(std::string error)
{
  /// TODO: Insert a list widget item that is colored (background).
  QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(error));
  item->setBackground(QBrush(QColor(238, 209, 212)));
  mListWidget->addItem(item);
  mListWidget->scrollToBottom();
}


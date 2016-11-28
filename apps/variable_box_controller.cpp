#include "variable_box_controller.h"
#include "constant.h"
#include <assert.h>

/* ContentViewController */

VariableBoxController::ContentViewController::ContentViewController(Responder * parentResponder, Context * context) :
  ViewController(parentResponder),
  m_context(context),
  m_firstSelectedRow(0),
  m_previousSelectedRow(0),
  m_currentPage(Page::RootMenu),
  m_selectableTableView(SelectableTableView(this, this))
{
}

const char * VariableBoxController::ContentViewController::title() const {
  return "Variable";
}

View * VariableBoxController::ContentViewController::view() {
  return &m_selectableTableView;
}

void VariableBoxController::ContentViewController::didBecomeFirstResponder() {
  m_selectableTableView.reloadData();
  m_selectableTableView.selectCellAtLocation(0, m_firstSelectedRow);
  app()->setFirstResponder(&m_selectableTableView);
}

bool VariableBoxController::ContentViewController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Back) {
    if (m_currentPage == Page::RootMenu) {
      m_firstSelectedRow = 0;
      app()->dismissModalViewController();
      return true;
    }
    m_selectableTableView.deselectTable();
    m_firstSelectedRow = m_previousSelectedRow;
    m_currentPage = Page::RootMenu;
    app()->setFirstResponder(this);
    return true;
  }
  if (event == Ion::Events::OK) {
    int selectedRow = m_selectableTableView.selectedRow();
    m_selectableTableView.deselectTable();
    if (m_currentPage == Page::RootMenu) {
      m_previousSelectedRow = selectedRow;
      m_firstSelectedRow = 0;
      m_currentPage = pageAtIndex(selectedRow);
      app()->setFirstResponder(this);
      return true;
    }
    m_firstSelectedRow = 0;
    char label[3];
    putLabelAtIndexInBuffer(selectedRow, label);
    const char * editedText = label;
    m_textFieldCaller->insertTextAtLocation(editedText, m_textFieldCaller->cursorLocation());
    m_textFieldCaller->setCursorLocation(m_textFieldCaller->cursorLocation() + strlen(editedText));
    m_currentPage = Page::RootMenu;
    app()->dismissModalViewController();
    return true;
  }
  return false;
}

int VariableBoxController::ContentViewController::numberOfRows() {
  switch (m_currentPage) {
    case Page::RootMenu:
      return 3;
    case Page::Scalar:
      return Context::k_maxNumberOfScalarExpressions;
    case Page::List:
      return Context::k_maxNumberOfListExpressions;
    case Page::Matrix:
      return Context::k_maxNumberOfMatrixExpressions;
    default:
      return 0;
  }
}

TableViewCell * VariableBoxController::ContentViewController::reusableCell(int index, int type) {
  assert(type < 2);
  assert(index >= 0);
  if (type == 0) {
    assert(index < k_maxNumberOfDisplayedRows);
    return &m_leafCells[index];
  }
  assert(index < k_numberOfMenuRows);
  return &m_nodeCells[index];
}

int VariableBoxController::ContentViewController::reusableCellCount(int type) {
  assert(type < 2);
  if (type == 0) {
    return k_maxNumberOfDisplayedRows;
  }
  return k_numberOfMenuRows;
}

void VariableBoxController::ContentViewController::willDisplayCellForIndex(TableViewCell * cell, int index) {
  if (m_currentPage == Page::RootMenu) {
    const char * label = nodeLabelAtIndex(index);
    MenuListCell * myCell = (MenuListCell *)cell;
    myCell->setText(label);
    return;
  }
  VariableBoxLeafCell * myCell = (VariableBoxLeafCell *)cell;
  char label[3];
  putLabelAtIndexInBuffer(index, label);
  myCell->setLabel(label);
  const Expression * expression = expressionForIndex(index);
  if (m_currentPage == Page::Scalar) {
    myCell->displayExpression(false);
    char buffer[Constant::FloatBufferSizeInScientificMode];
    ((Float *)expression)->convertFloatToText(buffer, Constant::FloatBufferSizeInScientificMode, Constant::NumberOfDigitsInMantissaInScientificMode);
    myCell->setSubtitle(buffer);
    return;
  }
  myCell->displayExpression(true);
  if (expression) {
    //myCell->setExpression(expression->createLayout());
  // TODO: display the dimensgion of the list/matrice as subtitle
    /* char buffer[4];
     * buffer[0] = (Matrice *)expression->dim(0);
     * buffer[1] = 'x';
     * buffer[2] = (Matrice *)expression->dim(1)
     * buffer[3] = 0;
     * myCell->setSubtitle(buffer);*/
  } else {
    myCell->setSubtitle("Vide");
  }
}

KDCoordinate VariableBoxController::ContentViewController::rowHeight(int index) {
  if (m_currentPage == Page::RootMenu) {
    return k_nodeRowHeight;
  }
  if (m_currentPage == Page::Scalar) {
    return k_leafRowHeight;
  }
  const Expression * expression = expressionForIndex(index);
  if (expression) {
    KDCoordinate expressionHeight = expression->createLayout()->size().height();
    return expressionHeight;
  }
  return k_leafRowHeight;
}

KDCoordinate VariableBoxController::ContentViewController::cumulatedHeightFromIndex(int j) {
  int result = 0;
  for (int k = 0; k < j; k++) {
    result += rowHeight(k);
  }
  return result;
}

int VariableBoxController::ContentViewController::indexFromCumulatedHeight(KDCoordinate offsetY) {
  int result = 0;
  int j = 0;
  while (result < offsetY && j < numberOfRows()) {
    result += rowHeight(j++);
  }
  return (result < offsetY || offsetY == 0) ? j : j - 1;
}

int VariableBoxController::ContentViewController::typeAtLocation(int i, int j) {
  if (m_currentPage == Page::RootMenu) {
    return 1;
  }
  return 0;
}

const Expression * VariableBoxController::ContentViewController::expressionForIndex(int index) {
  if (m_currentPage == Page::Scalar) {
    const Symbol symbol = Symbol('A'+index);
    return m_context->expressionForSymbol(&symbol);
  }
  if (m_currentPage == Page::Matrix) {
    return nullptr;
  }
  if (m_currentPage == Page::List) {
    return nullptr;
  }
  return nullptr;
}

VariableBoxController::ContentViewController::Page VariableBoxController::ContentViewController::pageAtIndex(int index) {
  switch (index) {
    case 0:
      return Page::Scalar;
    case 1:
      return Page::List;
    case 2:
      return Page::Matrix;
    default:
      assert(false);
      return Page::RootMenu;
  }
}

void VariableBoxController::ContentViewController::putLabelAtIndexInBuffer(int index, char * buffer) {
  if (m_currentPage == Page::Scalar) {
    buffer[0] = 'A' + index;
    buffer[1] = 0;
  }
  if (m_currentPage == Page::List) {
    buffer[0] = 'L';
    buffer[1] = '0' + index;
    buffer[2] = 0;
  }
  if (m_currentPage == Page::Matrix) {
    buffer[0] = 'M';
    buffer[1] = '0' + index;
    buffer[2] = 0;
  }
}

const char * VariableBoxController::ContentViewController::nodeLabelAtIndex(int index) {
  assert(m_currentPage == Page::RootMenu);
  switch (index) {
    case 0:
      return "Nombre";
    case 1:
      return "Liste";
    case 2:
      return "Matrice";
    default:
      assert(false);
      return nullptr;
  }
}

void VariableBoxController::ContentViewController::setTextFieldCaller(TextField * textField) {
  m_textFieldCaller = textField;
}

VariableBoxController::VariableBoxController(Context * context) :
  StackViewController(nullptr, &m_contentViewController, true, KDColorWhite, Palette::BoxTitleBackgroundColor, Palette::BoxTitleBackgroundColor),
  m_contentViewController(ContentViewController(this, context))
{
}

void VariableBoxController::didBecomeFirstResponder() {
  app()->setFirstResponder(&m_contentViewController);
}

void VariableBoxController::setTextFieldCaller(TextField * textField) {
  m_contentViewController.setTextFieldCaller(textField);
}
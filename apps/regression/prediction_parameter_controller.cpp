#include "prediction_parameter_controller.h"
#include <assert.h>

namespace Regression {

PredictionParameterController::PredictionParameterController(Responder * parentResponder, Store * store) :
  ViewController(parentResponder),
  m_selectableTableView(SelectableTableView(this, this, Metric::TopMargin, Metric::RightMargin,
    Metric::BottomMargin, Metric::LeftMargin)),
  m_goToParameterController(GoToParameterController(this, store))
{
}

const char * PredictionParameterController::title() const {
  return "Droite de regression";
}

View * PredictionParameterController::view() {
  return &m_selectableTableView;
}

void PredictionParameterController::didBecomeFirstResponder() {
  m_selectableTableView.selectCellAtLocation(0, 0);
  app()->setFirstResponder(&m_selectableTableView);
}

bool PredictionParameterController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK) {
    m_goToParameterController.setXPrediction(m_selectableTableView.selectedRow() == 0);
    StackViewController * stack = (StackViewController *)parentResponder();
    stack->push(&m_goToParameterController);
    return true;
  }
  return false;
}

int PredictionParameterController::numberOfRows() {
  return k_totalNumberOfCells;
};

TableViewCell * PredictionParameterController::reusableCell(int index) {
  assert(index >= 0);
  assert(index < k_totalNumberOfCells);
  return &m_cells[index];
}

int PredictionParameterController::reusableCellCount() {
  return k_totalNumberOfCells;
}

KDCoordinate PredictionParameterController::cellHeight() {
  return Metric::ParameterCellHeight;
}

void PredictionParameterController::willDisplayCellForIndex(TableViewCell * cell, int index) {
  ChevronMenuListCell * myCell = (ChevronMenuListCell *)cell;
  const char * titles[3] = {"Prediction sachant x", "Prediction sachant y"};
  myCell->setText(titles[index]);
}

}

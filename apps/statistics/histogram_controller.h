#ifndef STATISTICS_HISTOGRAM_CONTROLLER_H
#define STATISTICS_HISTOGRAM_CONTROLLER_H

#include <escher.h>
#include "store.h"
#include "histogram_view.h"
#include "histogram_parameter_controller.h"
#include "../curve_view.h"

namespace Statistics {

class HistogramController : public ViewController, public HeaderViewDelegate, public AlternateEmptyViewDelegate {

public:
  HistogramController(Responder * parentResponder, HeaderViewController * headerViewController, Store * store);
  const char * title() const override;
  View * view() override;
  StackViewController * stackController();
  HistogramParameterController * histogramParameterController();
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;

  int numberOfButtons() const override;
  Button * buttonAtIndex(int index) override;

  bool isEmpty() override;
  const char * emptyMessage() override;
  Responder * defaultController() override;
private:
  Responder * tabController() const;
  HistogramView m_view;
  Button m_settingButton;
  Store * m_store;
  uint32_t m_storeVersion;
  HistogramParameterController m_histogramParameterController;
};

}


#endif

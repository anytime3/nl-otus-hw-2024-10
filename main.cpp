#include <memory>
#include "libs/Controller.h"
#include "libs/Document.h"
#include "libs/View.h"
#include "libs/Primitive.h"


int main() {
  std::shared_ptr<Document> document = std::make_shared<Document>();
  std::shared_ptr<View> view = std::make_shared<View>(document);
  std::shared_ptr<Controller> controller = std::make_shared<Controller>(document, view);

  controller->createNewDocument();
  controller->importDocument("hw.pages");
  controller->exportDocument("hw_done.pages");
  controller->addPrimitive(std::make_shared<Circle>());
  controller->addPrimitive(std::make_shared<Rectangle>());
  controller->removePrimitive(std::make_shared<Rectangle>());
  view->display();
}

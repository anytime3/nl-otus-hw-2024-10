#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <memory>
#include "Document.h"
#include "View.h"

class Controller {
public:
  Controller(std::shared_ptr<Document> document, std::shared_ptr<View> view);
  void createNewDocument();
  void importDocument(const std::string & filename);
  void exportDocument(const std::string & filename) const;
  void addPrimitive(std::shared_ptr<Primitive> primitive);
  void removePrimitive(std::shared_ptr<Primitive> primitive);

private:
  std::shared_ptr<Document> document;
  std::shared_ptr<View> view;
};

#endif // CONTROLLER_H

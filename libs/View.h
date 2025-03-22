#ifndef VIEW_H
#define VIEW_H

#include <memory>
#include "Document.h"

class View {
public:
  View(std::shared_ptr<Document> document);
  void display() const;

private:
  std::shared_ptr<Document> document;
};

#endif // VIEW_H

#include "Controller.h"


Controller::Controller(std::shared_ptr<Document> document, std::shared_ptr<View> view)
{

}
void Controller::createNewDocument()
{
  std::cout << "Created new document" << std::endl;
}
void Controller::importDocument(const std::string & filename)
{
  std::cout << "Imported document " << filename << std::endl;
}
void Controller::exportDocument(const std::string & filename) const
{
  std::cout << "Exported document " << filename << std::endl;
}
void Controller::addPrimitive(std::shared_ptr<Primitive> primitive)
{
  std::cout << "Added primitive" << std::endl;
}
void Controller::removePrimitive(std::shared_ptr<Primitive> primitive)
{
  std::cout << "Removed primitive" << std::endl;
}

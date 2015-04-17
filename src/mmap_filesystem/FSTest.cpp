#include "Filesystem.h"
#include <iostream>
#include <string>

int main(void) {
	Storage::Filesystem fs("meta.db", "data.db");
	File file = fs.load("test");
	std::cout << "Writing lorem ipsum to file '" << file.name << "'" << std::endl;
	std::string data("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integre quisque fringilla?Lorem nam pellentesque id porta condimentum himenaeos torquent congue, purus habitasse hac nec ut.Feugiat fringilla nisl ipsum duis adipiscing sagittis curae: mi ultricies accumsan cubilia curabitur magna praesent.Posuere Duis dictum.Tortor fringilla nibh lacinia commodo hac interdum. Dis senectus massa quam.Ornare posuere Commodo ridiculus!Inceptos ultrices class pharetra faucibus sollicitudin lacinia libero vehicula?Turpis nisi facilisi pretium quam dictum litora imperdiet...Justo posuere Arcu non urna diam magnis, primis molestie ad taciti litora... Lorem varius metus lobortis tellus pretium proin primis praesent a hendrerit platea.Duis Lacinia nisi ante elit quis tellus eget consequat bibendum.Sit et sed nullam.Egestas pulvinar dolor ipsum libero conubia habitant ad.Pulvinar nunc Parturient enim gravida etiam blandit; himenaeos mauris augue curabitur - proin vivamus at semper... Dictumst vitae arcu tempor primis litora?Aptent integre lorem sem quisque sit nostra sapien blandit: amet sagittis himenaeos curabitur; interdum semper.Class sollicitudin potenti?Eu feugiat aliquet dolor adipiscing congue pretium volutpat - hendrerit vulputate?Nunc nam Enim sit scelerisque cubilia diam velit?");
	fs.write(&file, data.c_str(), data.size());

	char *x = fs.read(&file);
	std::string out(x, file.size);
	std::cout << "Result should be lorem ipsum:" << std::endl << out << std::endl << std::endl;

	std::cout << "Overwriting file with something short." << std::endl;
	fs.write(&file, "Hello, World!", 13);
	char *y = fs.read(&file);
	std::string out2(y, file.size);
	std::cout << "Result should be 'Hello, World!':" << std::endl << out2 << std::endl;

	free(x);
	free(y);
	fs.shutdown();
}

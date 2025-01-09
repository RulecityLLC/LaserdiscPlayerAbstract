#ifndef MPO_DELETER_H
#define MPO_DELETER_H

#include <memory>
using std::shared_ptr;

class MpoDeleter
{
protected:

	// In each concrete class, insert "void DeleteInstance() { delete this; }"
	// I did some experimentation and found that this virtual method must indeed be implemented in each concrete class.
	// I could not find a way to properly call the concrete class's destructor by deleting from this MpoDeleter class.
	virtual void DeleteInstance() = 0;

	class deleter;
	friend class deleter;

	// THIS DELETE CODE FROM BOOST EXAMPLE
	class deleter
	{
	public:
		void operator()(MpoDeleter *p)
		{
			p->DeleteInstance();
		}
	};
	// END BOOST EXAMPLE

};

#endif // MPO_DELETER_H

=?= Tutorial 2: Object Model =?=
__________________________________________________
[left][tutorial[001_HelloWord | Previous: Hello Word]][/left] [right][tutorial[011_ObjectConfig | Next: Object config]][/right]

=== Objectif ===
:** Understand ewol basic object
:** Use ewol::Object correctly

== Basis of the Object ==

An object in Ewol is a simple class : [class[ewol::Object]] This object is the basis of all element in the ewol system.
This is designed to manage basis element of complexe structure:

:** Unique ID
:** Name
:** Config
:** Event generation and receving
:** Xml configuration
:** Delayed removing

[note]
Please do not compare with the gObject basic class...
[/note]


== Create an Object: ==

This is a really simple way to create an Object, simply generate a new on it :
[code style=c++]
	widget::Label* tmpObject = new widget::Label();
	if (tmpObject == NULL) {
		APPL_ERROR("An error occured");
		return;
	}
[/code]

The object register itself on the object manager, now it will have a specific Id and no name

Force the set of the name :

[code style=c++]
	tmpObject->setName("my widget name");
	APPL_INFO("We just create an Object with ID=" << tmpObject->getId() << " name='" << tmpObject->getName() << "'");
[/code]


== Remove an Object: ==

This is important to note that many element can have a reference on the Object.

And we did not use "ref-counting" for philosophic reasons ...

Then we need to use the fuction:
[b]removeObject()[/b] to remove the Object, This will notify avery object in the system that this 
specific object has been removed.


Then to remove an object call:
[code style=c++]
	tmpObject->removeObject();
[/code]

On every object we can have an herited function: [b]virtual void onObjectRemove(ewol::Object * _removeObject);[/b]


We need to implement this fuction to be notify an object is removed:
[code style=c++]
	void namespeceName::ClassName::onObjectRemove(ewol::Object * _removeObject) {
		if (_removeObject == m_object) {
			m_object = NULL;
			markToRedraw(); // set only for graphical object ...
		}
	}
[/code]

[note]
If you have well follow the idea, you will never declare an object in local, just use pointer on them.
[/note]

=== Particularity ===

An object can remove itself, just use the function:
[code style=c++]
	autoDestroy();
[/code]


== Retrieve an Object: ==

In Ewol this is possible to get a widget with his name.
This is really simple.

=== In an Object ===

Call a simple function define in the Object:

[code style=c++]
	#include <ewol/object/Manager.h>
	
	...
	
	ewol::Object* tmpObject = getObjectManager().get("name of the object");
	if (tmpObject == NULL) {
		APPL_ERROR("The Object does not exist");
	}
[/code]

=== Not in an Object ===

In this case, we need to get the context manager and after the object manager:

[code style=c++]
	#include <ewol/object/Manager.h>
	#include <ewol/context/Context.h>
	
	...
	
	ewol::Object* tmpObject = ewol::getContext().getObjectManager().get("name of the object");
	if (tmpObject == NULL) {
		APPL_ERROR("The Object does not exist");
	}
[/code]



== conclusion ==

If you follow these rules, you will not have memory leek and no segmentation fault on the ewol system.

[note]
To be sure that the name is unique, just add the current creator object Id in the name.
[/note]



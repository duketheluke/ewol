

include $(EWOL_FOLDER)/Build/Makefile.common.mk


PROJECT_PACKAGE=$(PROJECT_NAME)package
JAVA_FOLDER=src/com/$(PROJECT_VENDOR)/$(PROJECT_NAME)


all:
	@echo $(CADRE_HAUT_BAS)
	@echo $(CADRE_COTERS)
	@echo '
	@echo '
	@echo '
	@echo '
	@echo '
	@echo $(CADRE_COTERS)
	@echo $(CADRE_HAUT_BAS)
	
	@echo $(F_ROUGE)"
	@rm -rf src jni/ewolAndroidAbstraction.cpp
	@echo $(F_ROUGE)"
	@mkdir -p $(JAVA_FOLDER)/
	
	@echo $(F_ROUGE)"
	@cp $(EWOL_FOLDER)/Java/PROJECT_NAME.java $(JAVA_FOLDER)/$(PROJECT_NAME).java
	@echo $(F_ROUGE)"
	@sed -i "s|__PROJECT_VENDOR__|$(PROJECT_VENDOR)|" $(JAVA_FOLDER)/$(PROJECT_NAME).java
	@sed -i "s|__PROJECT_NAME__|$(PROJECT_NAME)|" $(JAVA_FOLDER)/$(PROJECT_NAME).java
	@sed -i "s|__PROJECT_PACKAGE__|$(PROJECT_PACKAGE)|" $(JAVA_FOLDER)/$(PROJECT_NAME).java
	
	@echo $(F_ROUGE)"
	@cp $(EWOL_FOLDER)/Java/ewolAndroidAbstraction.cpp jni/
	@echo $(F_ROUGE)"
	@sed -i "s|__PROJECT_VENDOR__|$(PROJECT_VENDOR)|" jni/ewolAndroidAbstraction.cpp
	@sed -i "s|__PROJECT_NAME__|$(PROJECT_NAME)|" jni/ewolAndroidAbstraction.cpp
	@sed -i "s|__PROJECT_PACKAGE__|$(PROJECT_PACKAGE)|" jni/ewolAndroidAbstraction.cpp
	
	@echo $(F_ROUGE)"
	cd $(PROJECT_NDK) ; NDK_PROJECT_PATH=$(PROJECT_PATH) NDK_MODULE_PATH=$(PROJECT_MODULE) ./ndk-build
	@echo $(F_ROUGE)"
	PATH=$(PROJECT_SDK)/tools/:$(PROJECT_SDK)/platform-tools/:$(PATH) ant -Dsdk.dir=$(PROJECT_SDK) debug
	@echo $(F_ROUGE)"
	@rm -rf src jni/ewolAndroidAbstraction.cpp

install: all
	@echo $(CADRE_HAUT_BAS)
	@echo '           INSTALL : $(F_VIOLET)./bin/$(PROJECT_NAME)-debug.apk$(F_NORMALE)'$(CADRE_COTERS)
	@echo $(CADRE_HAUT_BAS)
	@# $(PROJECT_SDK)/platform-tools/adb kill-server
	@# install application
	sudo $(PROJECT_SDK)/platform-tools/adb  install -r ./bin/$(PROJECT_NAME)-debug.apk

clean:
	@echo $(CADRE_HAUT_BAS)
	@echo '           CLEANING : bin libs gen obj'$(CADRE_COTERS)
	@echo $(CADRE_HAUT_BAS)
	rm -rf bin libs gen obj
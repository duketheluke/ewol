//
//  EAGLView.h
//  OpenGLBasics
//
//  Created by Charlie Key on 6/24/09.
//


#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

/*
 This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
 The view content is basically an EAGL surface you render your OpenGL scene into.
 Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
 */
@interface OpenglView : UIView {
    
@private
    CAEAGLLayer* _eaglLayer;
	/* The pixel dimensions of the backbuffer */
	GLint backingWidth;
	GLint backingHeight;
	
	EAGLContext *_context;
    GLuint _colorRenderBuffer;
    GLuint _depthRenderBuffer;
	
	/* OpenGL names for the renderbuffer and framebuffers used to render to this view */
	GLuint viewRenderbuffer, viewFramebuffer;
	
	/* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
	GLuint depthRenderbuffer;
	
	//NSTimer *animationTimer;
	NSTimeInterval animationInterval;
	
	CGPoint touchLocation;
}

@property (nonatomic) NSTimeInterval animationInterval;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView;


@end
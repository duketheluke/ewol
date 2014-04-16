/**
 * @author Edouard DUPIN
 *
 * @copyright 2011, Edouard DUPIN, all right reserved
 *
 * @license BSD v3 (see license file)
 */

#import <UIKit/UIKit.h>
#include "ewol/context/IOs/Interface.h"

#import <ewol/context/IOs/OpenglView.h>
#import <ewol/context/IOs/AppDelegate.h>
#include <ewol/context/IOs/Context.h>

@implementation AppDelegate

@synthesize window;
@synthesize glView;

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	
	//No need for nib
	window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
	glView = [[OpenglView alloc] initWithFrame:window.bounds];
	[window addSubview:glView];
	[window makeKeyAndVisible];
	// Create interface of ewol here ....
	NSLog(@"CREATE EWOL interface creation\n");
	IOs::createInterface();
	glView.animationInterval = 1.0 / 60.0;
	[glView startAnimation];
}
/*
 - (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
 {
 // Override point for customization after application launch.
 return YES;
 }
 */

- (void)applicationWillResignActive:(UIApplication *)application
{
	glView.animationInterval = 1.0 / 5.0;
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	glView.animationInterval = 1.0 / 60.0;
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
	// Create interface of ewol here ....
	NSLog(@"REMOVE EWOL interface destruction\n");
	IOs::releaseInterface();
}
/*
 - (void)dealloc {
 [window release];
 [glView release];
 [super dealloc];
 }
 */

@end

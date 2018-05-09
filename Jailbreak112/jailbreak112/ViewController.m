//
//  ViewController.m
//  jailbreak112
//
//  Created by Sem Voigtländer on 11/04/2018.
//  Copyright © 2018 Jailed Inc. All rights reserved.
//

#import "ViewController.h"
#import "jailbreak.h"
#import "reboot.h"
extern char* stdoutPath;

@interface ViewController ()
- (IBAction)rebootBtnTap:(id)sender;
@property (weak, nonatomic) IBOutlet UITextView *logView;

@end

@implementation ViewController


-(void)updateUI:(NSString*)contents{
    self.logView.text = contents;
    
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [NSTimer scheduledTimerWithTimeInterval:1.0f repeats:YES block:^(NSTimer *timer){
        
        NSString* contents_out = @"";
        contents_out = [[NSString alloc] initWithContentsOfFile:[NSString stringWithUTF8String:stdoutPath]];
        
        [self performSelectorOnMainThread:@selector(updateUI:) withObject:contents_out waitUntilDone:NO];
    }];
   
}

-(void)viewDidAppear:(BOOL)animated {
    [NSThread detachNewThreadWithBlock:^(void){
         jailbreak();
    }];
   
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

//removed due to innecessarity
- (IBAction)rebootBtnTap:(id)sender {
    reboot_device();
}
@end

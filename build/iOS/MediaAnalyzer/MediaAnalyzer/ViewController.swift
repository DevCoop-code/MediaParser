//
//  ViewController.swift
//  MediaAnalyzer
//
//  Created by HanGyo Jeong on 2020/05/19.
//  Copyright © 2020 HanGyoJeong. All rights reserved.
//

import UIKit


class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()

        var parser:mediaParser = mediaParser()
        let str:String = parser.sayHello()
        NSLog("hello \(str)")
    }


}


// swift-tools-version: 5.9
import PackageDescription

// Enveloppe SwiftUI pour le Tetris console de 2008.
//
// Le jeu reste ce qu'il est : un binaire terminal. SwiftTerm fournit un
// émulateur de terminal natif dans lequel on le lance, ce qui donne une
// vraie fenêtre macOS — sans réécrire une ligne de C++.
let package = Package(
    name: "TetrisApp",
    platforms: [.macOS(.v13)],
    dependencies: [
        .package(url: "https://github.com/migueldeicaza/SwiftTerm", from: "1.2.0")
    ],
    targets: [
        .executableTarget(
            name: "TetrisApp",
            dependencies: ["SwiftTerm"]
        )
    ]
)

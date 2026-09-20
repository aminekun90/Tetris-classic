// Fenêtre macOS native pour le Tetris console de 2008.
//
// Le jeu n'est pas réécrit : SwiftTerm émule un terminal, on y lance le
// binaire construit par CMake, et SwiftUI l'habille d'une fenêtre.
import SwiftUI
import AppKit
import SwiftTerm

// MARK: - Localisation du binaire

enum GameBinary {
    /// Cherche l'exécutable, d'abord dans le bundle (app distribuée),
    /// puis dans le build CMake du dépôt (développement).
    static func locate() -> URL? {
        let fm = FileManager.default

        if let bundled = Bundle.main.url(forResource: "Tetris", withExtension: nil),
           fm.isExecutableFile(atPath: bundled.path) {
            return bundled
        }

        // macos/.build/... -> on remonte jusqu'à la racine du dépôt.
        var dir = URL(fileURLWithPath: CommandLine.arguments[0])
            .resolvingSymlinksInPath()
            .deletingLastPathComponent()

        for _ in 0..<6 {
            let candidate = dir.appendingPathComponent("build/Tetris")
            if fm.isExecutableFile(atPath: candidate.path) { return candidate }
            dir = dir.deletingLastPathComponent()
        }

        let cwd = URL(fileURLWithPath: fm.currentDirectoryPath)
        for relative in ["build/Tetris", "../build/Tetris", "Tetris"] {
            let candidate = cwd.appendingPathComponent(relative)
            if fm.isExecutableFile(atPath: candidate.path) { return candidate }
        }
        return nil
    }
}

// MARK: - Terminal

// Le délégué est un objet distinct : LocalProcessTerminalView implémente déjà
// le protocole pour son propre compte, en hériter créerait un conflit.
final class TerminalCoordinator: NSObject, LocalProcessTerminalViewDelegate {
    var onExit: ((Int32?) -> Void)?

    func sizeChanged(source: LocalProcessTerminalView, newCols: Int, newRows: Int) {}
    func setTerminalTitle(source: LocalProcessTerminalView, title: String) {}
    func hostCurrentDirectoryUpdate(source: TerminalView, directory: String?) {}
    func processTerminated(source: TerminalView, exitCode: Int32?) { onExit?(exitCode) }
}

struct TerminalContainer: NSViewRepresentable {
    let executable: URL
    let onExit: (Int32?) -> Void

    func makeCoordinator() -> TerminalCoordinator {
        let c = TerminalCoordinator()
        c.onExit = onExit
        return c
    }

    func makeNSView(context: Context) -> LocalProcessTerminalView {
        let view = LocalProcessTerminalView(frame: .zero)
        view.processDelegate = context.coordinator

        // Le jeu lit ses scores et son module a.xm dans le répertoire courant.
        let workingDirectory = executable.deletingLastPathComponent().path

        view.startProcess(
            executable: executable.path,
            args: [],
            environment: Terminal.getEnvironmentVariables(termName: "xterm-256color"),
            execName: nil,
            currentDirectory: workingDirectory
        )
        return view
    }

    func updateNSView(_ nsView: LocalProcessTerminalView, context: Context) {}
}

// MARK: - Interface

struct ContentView: View {
    @State private var exitCode: Int32??
    private let binary = GameBinary.locate()

    var body: some View {
        Group {
            if let binary {
                TerminalContainer(executable: binary) { code in exitCode = .some(code) }
                    .overlay(alignment: .bottom) {
                        if let exitCode, case let code?? = Optional(exitCode) {
                            Text("Le jeu s'est terminé (code \(code)). Ferme la fenêtre.")
                                .font(.callout)
                                .padding(8)
                                .background(.thinMaterial, in: Capsule())
                                .padding(.bottom, 12)
                        }
                    }
            } else {
                missingBinary
            }
        }
        .frame(minWidth: 860, minHeight: 560)
    }

    private var missingBinary: some View {
        VStack(spacing: 12) {
            Text("Le binaire du jeu est introuvable")
                .font(.title3.weight(.semibold))
            Text("Construis-le d'abord depuis la racine du dépôt :")
                .foregroundStyle(.secondary)
            Text("cmake -B build -DCMAKE_BUILD_TYPE=Release\ncmake --build build")
                .font(.system(.callout, design: .monospaced))
                .textSelection(.enabled)
                .padding(10)
                .background(.quaternary, in: RoundedRectangle(cornerRadius: 8))
        }
        .padding(40)
    }
}

@main
struct TetrisApp: App {
    var body: some Scene {
        Window("Tetris-classic — 2008", id: "tetris") {
            ContentView()
        }
        .windowResizability(.contentMinSize)
    }
}

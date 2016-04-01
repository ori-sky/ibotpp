#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/container/vector.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
//#include <clang/CodeGen/CodeGenAction.h>
//#include <clang/Basic/DiagnosticOptions.h>
//#include <clang/Driver/Compilation.h>
//#include <clang/Driver/Driver.h>
//#include <clang/Driver/Tool.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
//#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
//#include <llvm/ADT/SmallString.h>
//#include <llvm/ExecutionEngine/ExecutionEngine.h>
//#include <llvm/ExecutionEngine/MCJIT.h>
//#include <llvm/IR/Module.h>
//#include <llvm/Support/FileSystem.h>
//#include <llvm/Support/Host.h>
//#include <llvm/Support/ManagedStatic.h>
//#include <llvm/Support/Path.h>
//#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include "ibotpp.hpp"
#include "config.h"

void worker_main(boost::shared_ptr<boost::asio::io_service> io_service) {
	io_service->run();
}

int main(int argc, char *argv[]) {
	boost::container::vector<const char *> args;
	args.emplace_back("test.cpp");

	llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diag_opts(new clang::DiagnosticOptions());
	auto diag_printer = boost::make_shared<clang::TextDiagnosticPrinter>(llvm::errs(), diag_opts.get());
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_ids(new clang::DiagnosticIDs());
	clang::DiagnosticsEngine diag_engine(diag_ids, diag_opts.get(), diag_printer.get());

	auto ci = boost::make_shared<clang::CompilerInvocation>();
	clang::CompilerInvocation::CreateFromArgs(*ci, args.data(), args.data() + args.size(), diag_engine);

	clang::CompilerInstance compiler;
	compiler.setInvocation(ci.get());

	auto io_service = boost::make_shared<boost::asio::io_service>();
	auto work = boost::make_shared<boost::asio::io_service::work>(*io_service);
	auto strand = boost::make_shared<boost::asio::io_service::strand>(*io_service);
	boost::thread_group workers;
	for(unsigned int w = 0; w < CONFIG_NUM_WORKERS; ++w) {
		workers.create_thread(boost::bind(&worker_main, io_service));
	}
	ibotpp bot(io_service, strand, CONFIG_HOST, CONFIG_PORT);

	work.reset();
	workers.join_all();
	return 0;
}

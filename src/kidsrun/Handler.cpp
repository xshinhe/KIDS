#include "Handler.h"

#include <chrono>

#include "kids/Kernel_Recorder.h"
#include "kids/ModelFactory.h"
#include "kids/Param.h"
#include "kids/RecorderIO.h"
#include "kids/SolverFactory.h"

//
#include "mpi_utils.h"

namespace PROJECT_NS {


Handler::Handler(const std::string& solver_name, const std::string& model_name) {
    model  = ModelFactory(model_name);
    solver = SolverFactory(solver_name, model);
};


int Handler::run(std::shared_ptr<Param>& PM) {
    std::string hdlr_str = PM->get_string("handler", LOC(), "");
    if (false) {
    } else if (hdlr_str == "parallel") {
        run_parallel(PM);
    } else if (hdlr_str == "single") {
        run_single(PM);
    } else if (hdlr_str == "single_mpi") {
        run_single_mpi(PM);
    } else if (hdlr_str == "sampling") {
        run_sampling(PM);
    } else if (hdlr_str == "help") {
        run_help(PM);
    } else if (hdlr_str == "help_param") {
        run_help_param(PM);
    } else if (hdlr_str == "help_dataset") {
        run_help_dataset(PM);
    } else {
        throw std::runtime_error("unknown handler type!");
    }
    return 0;
}

int Handler::run_single(std::shared_ptr<Param>& PM) {
    std::shared_ptr<DataSet> DS    = std::shared_ptr<DataSet>(new DataSet());
    auto                     begin = std::chrono::steady_clock::now();
    Status                   stat;
    {
        solver->setInputParam(PM);
        solver->setInputDataSet(DS);
        solver->initializeKernel(stat);
        solver->executeKernel(stat);
    }
    auto   end        = std::chrono::steady_clock::now();
    double total_time = static_cast<std::chrono::duration<double>>(end - begin).count();

    std::cout << solver->generateInformationString(total_time);
    std::cout << "Using total time " << total_time << " s\n";
    return 0;
}

int Handler::run_single_mpi(std::shared_ptr<Param>& PM) {
    std::shared_ptr<DataSet> DS = std::shared_ptr<DataSet>(new DataSet());
    Status                   stat;
    auto                     begin = std::chrono::steady_clock::now();
    {
        MPI_Guard guard{};
        MPI_Barrier(MPI_COMM_WORLD);

        solver->setInputParam(PM);
        solver->setInputDataSet(DS);
        solver->initializeKernel(stat);
        solver->executeKernel(stat);
    }
    auto   end        = std::chrono::steady_clock::now();
    double total_time = static_cast<std::chrono::duration<double>>(end - begin).count();

    std::cout << solver->generateInformationString(total_time);
    std::cout << "Using total time " << total_time << " s\n";
    return 0;
}

int Handler::run_parallel(std::shared_ptr<Param>& PM) {
    std::shared_ptr<DataSet> DS    = std::shared_ptr<DataSet>(new DataSet());
    auto                     begin = std::chrono::steady_clock::now();
    Status                   stat;
    {
        solver->setInputParam(PM);
        solver->setInputDataSet(DS);
        solver->initializeKernel(stat);  // required!!!

        // auto& corr       = Kernel_Recorder::get_correlation();
        // int   nframe     = corr.frame;
        // int   nsize      = corr.size;
        // int   total_size = corr.size * corr.frame;

        // Result corr_sum(corr);
        // Result corr_mpi(corr);

        int         M = PM->get_int("M", LOC(), 1);
        int         istart, iend;
        std::string directory   = PM->get_string("directory", LOC(), "default");
        bool        write_init  = PM->get_bool("write_init", LOC(), false);
        bool        write_final = PM->get_bool("write_final", LOC(), false);

        MPI_Guard guard{};
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Guard::range(0, M, istart, iend);

        if (MPI_Guard::rank == 0) std::cout << PM->repr() << std::endl;

        // collect init
        // if (write_init) {
        //     std::ofstream ofs(utils::concat(directory, "/init-mpi", MPI_Guard::rank, ".dat"));
        //     for (auto& v : corr.header) ofs << FMT(8) << v;
        //     ofs << std::endl;
        // }
        // // collect final
        // if (write_final) {
        //     std::ofstream ofs(utils::concat(directory, "/final-mpi", MPI_Guard::rank, ".dat"));
        //     for (auto& v : corr.header) ofs << FMT(8) << v;
        //     ofs << std::endl;
        // }

        for (int icycle = istart, icalc = 0; icycle < iend; ++icycle, ++icalc) {
            auto mid1 = std::chrono::steady_clock::now();


            stat.icalc = icycle;
            solver->initializeKernel(stat);
            solver->executeKernel(stat);
            // collect init
            // if (write_init) {
            //     std::ofstream ofs(utils::concat(directory, "/init-mpi", MPI_Guard::rank, ".dat"), std::ios::app);
            //     for (int i = 0; i < nsize; ++i) ofs << FMT(8) << corr.data[i];
            //     ofs << std::endl;
            //     ofs.close();
            // }

            // // collect final
            // if (write_final) {
            //     std::ofstream ofs(utils::concat(directory, "/final-mpi", MPI_Guard::rank, ".dat"), std::ios::app);
            //     for (int i = 0; i < nsize; ++i) ofs << FMT(8) << corr.data[(nframe - 1) * nsize + i];
            //     ofs << std::endl;
            //     ofs.close();
            // }

            // clear correlation information
            // for (int iframe = 0, i = 0; iframe < nframe; ++iframe) {
            //     bool valid = (corr.stat[iframe] == 1);
            //     corr_sum.stat[iframe] += valid ? 1 : 0;
            //     for (int isize = 0; isize < nsize; ++isize, ++i) {
            //         corr_sum.data[i] += valid ? corr.data[i] : 0.0e0;
            //         corr.data[i] = 0.0e0;
            //     }
            //     corr.stat[iframe] = 0;
            // }

            auto mid2 = std::chrono::steady_clock::now();
            if (icycle == istart && MPI_Guard::rank == 0) {
                std::cout << "expect time: "
                          << static_cast<std::chrono::duration<double>>(mid2 - mid1).count() * (iend - istart)  //
                          << std::endl;
            }
        }
        // MPI_Reduce(corr_sum.stat.data(), corr_mpi.stat.data(), corr.frame, MPI::INT, MPI_SUM, 0, MPI_COMM_WORLD);
        // MPI_Reduce(corr_sum.data.data(), corr_mpi.data.data(), total_size, MPI::DOUBLE_PRECISION, MPI_SUM, 0,
        //            MPI_COMM_WORLD);
        // MPI_Barrier(MPI_COMM_WORLD);

        // for (int iframe = 0, i = 0; iframe < nframe; ++iframe) {
        //     for (int isize = 0; isize < nsize; ++isize, ++i) corr_sum.data[i] /= corr_sum.stat[iframe];
        // }
        // // corr_sum.save(utils::concat(directory, "/corr-mpi", MPI_Guard::rank, ".dat"), 0, -1, true); // @debug
        // if (MPI_Guard::isroot) {
        //     for (int iframe = 0, i = 0; iframe < nframe; ++iframe) {
        //         for (int isize = 0; isize < nsize; ++isize, ++i) corr_mpi.data[i] /= corr_mpi.stat[iframe];
        //     }
        //     corr_mpi.save(utils::concat(directory, "/corr.dat"), 0, -1, true);
        // }
    }
    auto   end        = std::chrono::steady_clock::now();
    double total_time = static_cast<std::chrono::duration<double>>(end - begin).count();

    // report time cost
    if (MPI_Guard::isroot) {
        std::cout << solver->generateInformationString(total_time);
        std::cout << "Using total time " << total_time << " s\n";
        RecorderIO::flush_all();
    }
    return 0;
}

int Handler::run_parallel_22(std::shared_ptr<Param>& PM) {
    std::shared_ptr<DataSet> DS    = std::shared_ptr<DataSet>(new DataSet());
    auto                     begin = std::chrono::steady_clock::now();
    Status                   stat;
    {
        solver->setInputParam(PM);
        solver->setInputDataSet(DS);
        solver->initializeKernel(stat);  // required!!!

        int       M = PM->get_int("M", LOC(), 1);
        int       istart, iend;
        MPI_Guard guard{};
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Guard::range(0, M, istart, iend);

        if (MPI_Guard::rank == 0) std::cout << PM->repr() << std::endl;

        for (int icycle = istart, icalc = 0; icycle < iend; ++icycle, ++icalc) {
            stat.icalc = icycle;
            solver->initializeKernel(stat);
            solver->executeKernel(stat);
            Kernel::dump(solver.get());
            Kernel::report(solver.get());
        }
        // MPI_Guard::reduce(Kernel::mpi_reduce_info(solver.get()));
        MPI_Barrier(MPI_COMM_WORLD);

        // Kernel::result(solver);
    }
    auto   end        = std::chrono::steady_clock::now();
    double total_time = static_cast<std::chrono::duration<double>>(end - begin).count();

    // report time cost
    if (MPI_Guard::isroot) {
        std::cout << solver->generateInformationString(total_time);
        std::cout << "Using total time " << total_time << " s\n";
    }
    return 0;
}


int Handler::run_sampling(std::shared_ptr<Param>& PM) {
    std::shared_ptr<DataSet> DS    = std::shared_ptr<DataSet>(new DataSet());
    auto                     begin = std::chrono::steady_clock::now();
    Status                   stat;
    {
        solver->setInputParam(PM);
        solver->setInputDataSet(DS);
        solver->initializeKernel(stat);

        int M = PM->get_int("M", LOC(), 1);
        int istart, iend;

        MPI_Guard guard{};
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Guard::range(0, M, istart, iend);

        if (MPI_Guard::rank == 0) std::cout << PM->repr() << std::endl;

        for (int icycle = istart, icalc = 0; icycle < iend; ++icycle, ++icalc) {
            auto mid1  = std::chrono::steady_clock::now();
            stat.icalc = icycle;
            solver->initializeKernel(stat);
        }
    }
    auto   end        = std::chrono::steady_clock::now();
    double total_time = static_cast<std::chrono::duration<double>>(end - begin).count();

    // report time cost
    if (MPI_Guard::isroot) {
        std::cout << solver->generateInformationString(total_time);
        std::cout << "Using total time " << total_time << " s\n";
    }
    return 0;
}

int Handler::run_help_dataset(std::shared_ptr<Param>& PM) {
    {
        for (auto&& i : VARIABLE_BASE::_LIST) {  //
            std::cout << i->name() << ", " << i->doc() << "\n";
        }
    }
    return 0;
}


};  // namespace PROJECT_NS

cmd_/home/chn/CS353/project1/calc/Module.symvers := sed 's/\.ko$$/\.o/' /home/chn/CS353/project1/calc/modules.order | scripts/mod/modpost -m -a  -o /home/chn/CS353/project1/calc/Module.symvers -e -i Module.symvers   -T -

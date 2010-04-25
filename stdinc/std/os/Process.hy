import std.os.process;

class Process {
	protected pd, execName, arg;
	protected isPipe = -1;
	
	public method Process ( execName, arg ){
		me->execName = execName;
		me->arg = arg;
	}
	public method Process ( execName ){
		me->execName = execName;
		me->arg = "";
	}
	public method Process (){
		me->execName = "";
		me->arg = "";
	}
	public method close (){
		if ( me->isPipe ){
			pclose( pd );
			me->isPipe = -1;
		}
	}
	public method setexecName ( execName ){
		me->close();
		me->execName = execName;
	}
	public method setArg ( arg ){
		me->close();
		me->execName = arg;
	}
	private method _exec( execName, arg ){
		if ( execName == "" ) {
			return -1;
		}
		me->isPipe = -1;
		exec( execName, arg );
	}
	public method exec(){
		me->_exec( me->execName, me->arg );
	}
	public method exec ( execName, arg ){
		me->_exec( execName, arg );
	}
	private method _pipeOpen( execName, arg, mode ){
		if ( ( execName == "") | ( mode == "") ) {
			return -1;
		}
		me->pd = popen( execName." ".arg, mode );
		me->isPipe = 1;
	}
	public method pipeOpen ( mode ){
		me->_pipeOpen( me->execName, me->arg, mode);
	}
	public method pipeOpen ( execName, mode ){
		me->_pipeOpen( execName, me->arg, mode );
	}
	public method pipeOpen ( execName, arg, mode ){
		me->_pipeOpen( execName, arg, mode );
	}
}			
